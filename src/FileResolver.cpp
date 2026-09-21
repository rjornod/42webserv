#include "FileResolver.hpp"

// Builds the file system path based on the uri and the location context
// First it decodes escaped hex characters such as %2E -> '.' from the uri
// Then it checks whether the decoded uri tries to escape the main directory
// for safety of the host of the webserver
Result<std::filesystem::path, FileResolutionError> FileResolver::resolve(const RequestContext& ctx){

  std::string fSPString;
  std::string locationRoot = ctx.getLocationConfig()->getRoot();
  HttpRequest request = ctx.getHttpRequest();
  std::string_view raw = request.getURI();

  auto decodedUri = percentDecode(raw);

  if (!decodedUri) {
    switch (decodedUri.error()) {
      case UriDecodeError::MALFORMED_PERCENT_ENCODING:
        std::cout << "Invalid URI: " << to_string(decodedUri.error()) << std::endl;
        return Result<std::filesystem::path, FileResolutionError>::Err(
          FileResolutionError::BAD_REQUEST);
        // break;
      case UriDecodeError::INVALID_BYTE:
        std::cout << "Invalid URI: " << to_string(decodedUri.error()) << std::endl;
        return Result<std::filesystem::path, FileResolutionError>::Err(
          FileResolutionError::BAD_REQUEST);
        // break;
      default:
        return Result<std::filesystem::path, FileResolutionError>::Err(
          FileResolutionError::BAD_REQUEST);
    }
  }

  auto segments = normalizeSegments(decodedUri.value());

  if (!segments) {
    std::cout << "Invalid URI segments: " << to_string(segments.error()) << std::endl;
    return Result<std::filesystem::path, FileResolutionError>::Err(
      FileResolutionError::FORBIDDEN);
  }

  fSPString = makeFSPath(locationRoot, segments.value());
  // std::cout << fSPString << std::endl;

  std::filesystem::path candidate = fSPString;
  std::filesystem::path root = locationRoot;
  Result<std::filesystem::path, FileResolutionError> path = checkWithinRoot(candidate, root);
  if (!path)
    return Result<std::filesystem::path, FileResolutionError>::Err(path.error());

  // Now check whether the file is a dirctory
  auto status = std::filesystem::status(path.value());
  if (std::filesystem::is_regular_file(status))
    return Result<std::filesystem::path, FileResolutionError>::Ok(path.value());
  else if (std::filesystem::is_directory(status)) {
    auto indexResult = resolveIndex(path.value(), ctx.getLocationConfig()->getIndex());
    if (!indexResult)
      return Result<std::filesystem::path, FileResolutionError>::Err(FileResolutionError::FORBIDDEN);
    return Result<std::filesystem::path, FileResolutionError>::Ok(indexResult.value());
  }
  return Result<std::filesystem::path, FileResolutionError>::Err(FileResolutionError::FORBIDDEN);
}

bool is_hex(char c) {
  return (c >= '0' && c <= '9') ||
        (c >= 'A' && c <= 'F') ||
        (c >= 'a' && c <= 'f');
}

// Converts one hex digit character ('0'-'9', 'a'-'f', 'A'-'F') to its numeric value (0-15)
int hex_digit_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1; // shouldn't happen if is_hex was checked
}

// Combines two hex digit characters into one byte, e.g. hex_to_char('2','0') -> ' ' (space)
char hex_pair_to_byte(char high, char low) {
    int value = hex_digit_value(high) * 16 + hex_digit_value(low);
    return static_cast<char>(value);
}

bool isDangerousByte(char c) {

    unsigned char uc = static_cast<unsigned char>(c);

    // NULL byte
    if (uc == 0x00) return true;

    // other C0 control characters (0x01–0x1F) — things like \n, \r, tab
    // these can be used to inject fake headers or otherwise confuse
    // downstream parsers/logs that assume a path is "normal" text
    if (uc < 0x20) return true;

    // DEL character
    if (uc == 0x7F) return true;

    return false;
}

Result<std::string, UriDecodeError> FileResolver::percentDecode(std::string_view raw) {
  
  std::string out;

  for (size_t i = 0; i < raw.size(); i++) {
    char c = raw[i];
    if (c == '%') {
      if (i + 2 >= raw.size() || !is_hex(raw[i + 1]) || !is_hex(raw[i + 2]))
        return Result<std::string, UriDecodeError>::Err(UriDecodeError::MALFORMED_PERCENT_ENCODING);
      c = hex_pair_to_byte(raw[i + 1], raw[i + 2]);
      i += 2;
    }
    if (isDangerousByte(c))
      return Result<std::string, UriDecodeError>::Err(UriDecodeError::INVALID_BYTE);
    out.push_back(c);
  }

  return Result<std::string, UriDecodeError>::Ok(std::move(out));
}

std::vector<std::string_view> splitUri(std::string_view uri) {

  std::vector<std::string_view> segments;

  size_t start = 0;
  while (start <= uri.size()) {
    size_t slashPos = uri.find('/', start);

    if (slashPos == std::string_view::npos) { // No more slashes, take everything
      segments.push_back(uri.substr(start));
      break;
    }

    // Add the piece between the start and the slash just found
    segments.push_back(uri.substr(start, slashPos - start));

    start = slashPos + 1;
  }

  return segments;
}


//Careful! Stack is made of string_views -- don't let it live further than
// decodedURI, before using the result
Result<std::vector<std::string_view>, PathResolutionError> FileResolver::normalizeSegments(std::string_view decodedUri) {
  
  std::vector<std::string_view> rawSegments = splitUri(decodedUri);
  std::vector<std::string_view> stack;

  for (std::string_view segment : rawSegments) {
    if (segment.empty() || segment == ".") {
      // ignore: empty means "//" (double slash), "." means "stay here"
      continue;
    }
    else if (segment == "..") {
      if (!stack.empty()) 
        stack.pop_back(); // We already went down at least once -- go one up
      else {
        //Trying to escape the root -- return an error
        return Result<std::vector<std::string_view>, PathResolutionError>::Err(
            PathResolutionError::PATH_ESCAPES_ROOT);
      }
    }
    else 
      stack.push_back(segment);
  }

  return Result<std::vector<std::string_view>, PathResolutionError>::Ok(
    std::move(stack));
}

//Joins two paths properly, i.e., by avoiding double slashes
std::string joinPath(std::string_view base, std::string_view suffix) {
    std::string result;
    result.reserve(base.size() + suffix.size() + 1);

    // strip trailing slash from base, if present
    if (!base.empty() && base.back() == '/')
        base.remove_suffix(1);

    // strip leading slash from suffix, if present
    if (!suffix.empty() && suffix.front() == '/')
        suffix.remove_prefix(1);

    result.append(base);
    result.push_back('/');
    result.append(suffix);

    return result;
}

std::string FileResolver::makeFSPath(std::string root, std::vector<std::string_view> segments) {

  std::string path = root;

  for (auto segment : segments)
    path = joinPath(path, segment);

  return path;
}

/* This function checks against the file systemwhether the path resolved from
 the text is actually contained in the root; it then also catches whether
 the file and the root exist
 */
// Mismatch checks the elements in two iterators pair by pair and return the first
// pair that is not equal. If everything is the same (assuming the first it is
// potentially shorter), returns the end of the first iterator. In this case, we
// compare the absolute paths both of the root and of the request resolved file path
// If the path of the root is not fully contained in the candidate (i.e. mismatch
// returns other than the end of the root), it means the path is somewhere outside
// the root directory
Result<std::filesystem::path, FileResolutionError> FileResolver::checkWithinRoot(const std::filesystem::path& candidate, const std::filesystem::path& root) {
  
  std::error_code ec;

  std::filesystem::path canonical_root = std::filesystem::canonical(root, ec);

  if (ec)
    // Error code is set
    // Meaning root doesn't exist
    // This is a server config problem
    return Result<std::filesystem::path, FileResolutionError>::Err(
      FileResolutionError::SERVER_ERROR);

  std::filesystem::path canonical_path = std::filesystem::canonical(candidate, ec);

  if (ec)
    // Error code is set
    // Meaning that the file (or some part along the way) doesn't exist
    // File not found
    return Result<std::filesystem::path, FileResolutionError>::Err(
      FileResolutionError::NOT_FOUND);

  // Now check if canonical_root is a prefix (in filesystem terms) of canonical_path
  // Meaning that the path escapes the root
  auto [root_end, nothing] = std::mismatch(canonical_root.begin(),
                                          canonical_root.end(),
                                          canonical_path.begin());
                                        
  if (root_end != canonical_root.end())
    // Means that the path escaped the root
    return Result<std::filesystem::path, FileResolutionError>::Err(
      FileResolutionError::FORBIDDEN);

  return Result<std::filesystem::path, FileResolutionError>::Ok(
    std::move(canonical_path));

}

/* Checks against the filesystem whether directory/location.indexCandidate exists
 returns the first index candidate that exists -- if none exists, return 403 (or 404)
*/
Result<std::filesystem::path, FileResolutionError> FileResolver::resolveIndex(
  const std::filesystem::path& directory,
  const std::vector<std::string>& indexCandidates) {

  for (const auto& name : indexCandidates) {
    std::filesystem::path path = directory / name;

    std::error_code ec;
    if (std::filesystem::is_regular_file(path) && !ec)
      return Result<std::filesystem::path, FileResolutionError>::Ok(path);
  }

  //If none of the index candidates exist, return an error
  return Result<std::filesystem::path, FileResolutionError>::Err(FileResolutionError::FORBIDDEN);
}