#include "FileResolver.hpp"
#include "URIError.hpp"
#include "Result.hpp"


std::string FileResolver::resolve(const RequestContext& ctx){

  std::string locationRoot = ctx.getLocationConfig()->getRoot();

  std::string_view raw = ctx.getHttpRequest().getURI();
  auto decodedUri = percentDecode(raw);
  if (!decodedUri) {
    switch (decodedUri.error()) {
      case URIError::MALFORMEDPERCENTENCODING:
        return "";
        break;
      case URIError::ESCAPESROOT:
        return "";
        break;
      case URIError::INVALIDBYTE:
        return "";
        break;
      case URIError::UNKNOWN:
        return "";
        break;
      default:
        return "";
    }
  }

  // std::string rootMinusSlash = locationRoot.erase(0, 1);

  // std::string filePath = rootMinusSlash + ctx.getHttpRequest().getURI();

  // return uri;
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

// For now it just copies the string_view 
Result<std::string, URIError> FileResolver::percentDecode(std::string_view raw) {
  std::string out;

  for (size_t i = 0; i < raw.size(); i++) {
    if (raw[i] == '%') {
      if (i + 2 >= raw.size() || !is_hex(raw[i + 1]) || !is_hex(raw[i + 2]))
        return Result<std::string, URIError>::Err(URIError::MALFORMEDPERCENTENCODING);
      out.push_back(hex_pair_to_byte(raw[i + 1], raw[i + 2]));
      i += 2;
    }
    else
      out.push_back(raw[i]);
  }

  std::vector<std::string_view> stack;



  return Result<std::string, URIError>::Ok(std::move(out));
}


//Careful! Stack is made of string_views -- don't let it live further than
// decodedURI, before using the result
Result<std::vector<std::string_view>, URIError> normalize_segments(std::string_view decodedUri) {
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
        return Result<std::vector<std::string_view>, URIError>::Err(
            URIError::ESCAPESROOT);
      }
    }
    else 
      stack.push_back(segment);
  }

  return Result<std::vector<std::string_view>, URIError>::Ok(std::move(stack));
}