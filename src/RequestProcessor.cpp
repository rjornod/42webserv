#include "RequestProcessor.hpp"
#include <algorithm>
#include <sstream>


static int statusFromError(FileResolutionError e) {
  switch (e) {
    case FileResolutionError::BAD_REQUEST: return 400;
    case FileResolutionError::FORBIDDEN:   return 403;
    case FileResolutionError::NOT_FOUND:   return 404;
    case FileResolutionError::SERVER_ERROR:
    default:                               return 500;
  }
}


HttpResponse RequestProcessor::process(RequestContext& ctx){

  std::cout << "Processing request" << std::endl;
  if (ctx.getHttpRequest().getBodySize() > ctx.getClientBodySize())
    return buildErrorResponse(413, ctx);
  if (!allowedMethod(ctx))
    return buildErrorResponse(405, ctx);

  switch (ctx.getHttpRequest().getMethod())
  {
  case HttpMethod::GET: return handleGet(ctx);
  case HttpMethod::POST: return handlePost(ctx);
  case HttpMethod::DELETE: return handleDelete(ctx);
  default: return buildErrorResponse(405, ctx);
  }

}

//Resolves the path and handles the request
HttpResponse RequestProcessor::handleGet(RequestContext& ctx) {

  Resolution res;
  HttpResponse err;

  if (!resolvePath(ctx, res, err))
    return err;

  if (res.kind == Resolution::DIRECTORY)
    return buildAutoIndexResponse(ctx);
  return staticHandler(ctx);
}

//Resolves the path and handles the request
HttpResponse RequestProcessor:: handleDelete(RequestContext& ctx) {

  Resolution res;
  HttpResponse errResponse;
  FileResolver fileResolver;

  Result<Resolution, FileResolutionError> pathResult = fileResolver.resolveForWrite(ctx);

  if (!pathResult)
    return buildErrorResponse(statusFromError(pathResult.error()), ctx);

  res = pathResult.value();
  if (res.kind == Resolution::DIRECTORY)
    return buildErrorResponse(409, ctx);
  ctx.setFilePath(res.path);
  return deleteHandler(ctx);
}

HttpResponse RequestProcessor::handlePost(RequestContext& ctx) {

  const HttpRequest& req = ctx.getHttpRequest();

  if (!postHeadersPresent(req))
    return buildErrorResponse(411, ctx);
  if (!checkContentLength(ctx))
    return buildErrorResponse(413, ctx);

  std::unordered_map<std::string, std::string> params;
  std::string type;

  //TO DO parse the params 
  if (!parseContentType(req, params, type)) 
    return buildErrorResponse(400, ctx);
  if (!checkSupportedMedia(type))
    return buildErrorResponse(415, ctx);
  
  if (type == "multipart/form-data")
    return buildErrorResponse(500, ctx); // For now... TO DO: handle multipart

  return handleRawUpload(ctx);


  return HttpResponse();
}

//TO DO: return a FileSystem error instead of an int in case of error
int writeFileAtomically(const std::filesystem::path& target, const std::string& data) {

  // Temp file in the SAME directory, so rename() stays on one filesystem (atomic)
  std::filesystem::path tmp = target.parent_path() / (".tmp_" + target.filename().string() + "_"); // TO DO: add + uniqueSuffix()

  {
    std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
    if (!out.is_open())
      return errno;                  // permission denied, no space for the inode, ...
    out.write(data.data(), static_cast<std::streamsize>(data.size()));
    out.close();
    if (!out) {                      // write or flush failed (e.g. disk full)
      std::error_code ignored;
      std::filesystem::remove(tmp, ignored);
      return EIO;
    }
  }

  std::error_code ec;
  std::filesystem::rename(tmp, target, ec);
  if (ec) {
    std::error_code ignored;
    std::filesystem::remove(tmp, ignored);        // don't leave the temp file behind
    return ec.value();
  }
  return 0;
}

HttpResponse RequestProcessor::handleRawUpload(RequestContext& ctx) {
  const HttpRequest req = ctx.getHttpRequest();

  FileResolver fileResolver;
  auto target = fileResolver.resolveForWrite(ctx);
  if (!target)
    return buildErrorResponse(statusFromError(target.error()), ctx);
  
  // If the file exists we refuse to overwrite (POLICY)
  std::error_code ec;
  if (std::filesystem::exists(target.value().path, ec))
    return buildErrorResponse(409, ctx);
  
  // Write the body to disk
  int err = writeFileAtomically(target.value().path, req.getBody());
  if (err != 0) {
    std::cout << "Some error occured in writeFileAtomically" << std::endl;
    return buildErrorResponse(statusFromError(FileResolutionError::BAD_REQUEST), ctx); // TO DO: introduce here the proper error to send; for now 400
  }
  // 4. 201 Created, with Location pointing at the new resource
  HttpResponse resp;
  resp.setStatusCode(201);
  resp.makeStatusLine();
  resp.addHeader("Location", req.getURI());
  resp.addHeader("Content-Length", "0");
  return resp;

}

bool RequestProcessor::checkSupportedMedia(std::string type) {

  std::unordered_set<std::string> supportedMedia = {"text/html", "text/css",
    "image/gif", "application/javascript",
    "text/plain", "application/json", "application/pdf",
    "multipart/form-data"};

  return supportedMedia.contains(type);

}


bool RequestProcessor::parseContentType(const HttpRequest& req, std::unordered_map<std::string, std::string>& params, std::string& type) {

  
  std::unordered_map<std::string, std::string> headers = req.getHeaders();
  auto it = headers.find("content-type");

  if (it == headers.end())
    return false;
  
  auto contentTypeHeader = it->second;

  size_t colon = contentTypeHeader.find(';');
  if (colon == std::string::npos)
    type = contentTypeHeader;
  else
    type = contentTypeHeader.substr(0, colon);
  std::for_each(type.begin(), type.end(), [](char& c) {c = tolower(c);});
 
  //TO DO trim from white space
  (void)params;
  return true;

}

//Returns true on success and fills out the response; on failure returns false and fills out errResponse
bool RequestProcessor::resolvePath(RequestContext& ctx, Resolution& out, HttpResponse& errResponse) {
  
  FileResolver fileResolver;

  Result<Resolution, FileResolutionError> pathResult = fileResolver.resolve(ctx);

  if (!pathResult) {
    errResponse = buildErrorResponse(statusFromError(pathResult.error()), ctx);
    return false;
  }

  out = pathResult.value();
  ctx.setFilePath(out.path);
  return true;
}

bool RequestProcessor::checkContentLength(const RequestContext& ctx) {

  int maxBodySize;
  if (ctx.getLocationConfig())
    maxBodySize = ctx.getLocationConfig()->getMaxBodySize();
  else
    maxBodySize = ctx.getServerConfig()->getClientMaxBody();
  
  std::unordered_map<std::string, std::string> headers = ctx.getHttpRequest().getHeaders();
  auto contentLen = headers.find("content-length");

  if (contentLen == headers.end())
    return true;
  try {
    int len = std::stoi(contentLen->second);
    return len <= maxBodySize;
  }
  catch (const std::out_of_range& oor) {
    return false;
  }

}

bool RequestProcessor::postHeadersPresent(const HttpRequest& req) {

  std::unordered_map<std::string, std::string> headers = req.getHeaders();
  auto contentLen = headers.find("content-length");
  auto transferEncoding = headers.find("transfer-encoding");
  return (contentLen == headers.end() || transferEncoding == headers.end());
}

bool RequestProcessor::allowedMethod(const RequestContext& ctx) {
  HttpMethod requestMethod = ctx.getHttpRequest().getMethod();


  std::vector<std::string> allowedMethods;
  if (ctx.getLocationConfig())
    allowedMethods = ctx.getLocationConfig()->getAllowedMethods();
  else
  //TODO change in ServerConfig
    allowedMethods.emplace_back("GET");

  if (std::find(allowedMethods.begin(), allowedMethods.end(), to_string(requestMethod)) 
      == allowedMethods.end()) {
        std::cout << "Method not allowed in the context" << std::endl;
        return false;
      }
  return true;

}

HttpResponse RequestProcessor::staticHandler(RequestContext& ctx) {
  
  const std::filesystem::path& path = ctx.getFilePath();
  
  HttpResponse response;
  response.makeStatusLine();
  response.addHeader("Content-Length", std::to_string(std::filesystem::file_size(path)));
  Result<std::string, MimeTypeError> mimeType = guessMimeType(path.string());
  if (!mimeType)
    response.addHeader("Content-Type", "application/octet-stream"); // IF im gonna handle the error like this, 
    //might as well refactor the function not to take return Result, 
    //but just return default value on not found type
  else
    response.addHeader("Content-Type", mimeType.value());
  response.setBodySource(path);

  return response;
}

const std::unordered_map<std::string, std::string> RequestProcessor::M_MIME_TYPES = {
  {"html", "text/html"},
  {"css", "text/css"},
  {"gif", "image/gif"},
  {"js", "application/javascript"},
  {"txt", "text/plain"},
  {"json", "application/json"},
  {"pdf", "application/pdf"},
};

Result<std::string, MimeTypeError> RequestProcessor::guessMimeType(std::string file) {

  std::string fileName = file;

  size_t dot = fileName.find(".");

  if (dot == std::string::npos)
    return Result<std::string, MimeTypeError>::Ok("application/octet-stream");

  std::string ext = fileName.substr(dot + 1);

  auto it = M_MIME_TYPES.find(ext);
  if (it == M_MIME_TYPES.end())
    return Result<std::string, MimeTypeError>::Err(MimeTypeError::EXTENSION_NOT_FOUND);

  return Result<std::string, MimeTypeError>::Ok(it->second);

}

// Lookup in the LocationConfig the error page
// Returns an optional that holds the string if found
const std::optional<std::string> RequestProcessor::findErrorPage(int errorCode, const RequestContext& ctx) {

  std::map<int, std::string> pages;
  if (ctx.getLocationConfig())
    pages = ctx.getLocationConfig()->getErrorPages();
  else
    pages = ctx.getServerConfig()->getErrorPages();

  auto it = pages.find(errorCode);
  if (it != pages.end())
    return it->second;
  return std::nullopt;
}


RequestContext RequestProcessor::makeErrorCtx(const RequestContext& ctx, const std::string& uri) {
  RequestContext errorCtx = ctx;

  HttpRequest& req = errorCtx.getHttpRequest();
  req.setMethod(HttpMethod::GET);
  req.setURI(uri);
  req.clearBody();
  // TODO Remove headers
  errorCtx.setFilePath({});
  return errorCtx;

}

HttpResponse RequestProcessor::buildErrorResponse(int errorCode, const RequestContext& ctx) {
  
  if (auto errorPage = findErrorPage(errorCode, ctx)) {
    
    
    RequestContext errorCtx = makeErrorCtx(ctx, *errorPage);

    FileResolver fileResolver;
    auto result = fileResolver.resolve(errorCtx);
    if (!result)
      std::cout << "error on resolving error page path " << to_string(result.error()) << std::endl;
    if (result && result.value().kind == Resolution::FILE) {
      errorCtx.setFilePath(result.value().path);
      HttpResponse response = staticHandler(errorCtx);
      response.updateStatus(errorCode);
      return response;
    }
  }
  return buildDefaultErrorResponse(errorCode);
  
}

HttpResponse RequestProcessor::buildDefaultErrorResponse(int errorCode) {

  HttpResponse response;

  response.setStatusCode(errorCode);
  response.makeStatusLine();
  std::string reason = response.getReasonPhrase();
  std::string pathStr = "./data/error_pages/";
  pathStr += std::to_string(errorCode) + ".html";
  std::error_code ec;
  std::filesystem::path path = std::filesystem::canonical(pathStr, ec);
  response.addHeader("Content-Type", "text/html");
  if (ec) {
    response.addHeader("Content-Length", std::to_string(std::to_string(errorCode).size() + reason.size() + 1));
    std::cout << std::to_string(errorCode) + " " + reason << std::endl;
    response.setBodySource(std::to_string(errorCode) + " " + reason);
  }
  else {
    response.addHeader("Content-Length", std::to_string(std::filesystem::file_size(path)));
    response.setBodySource(path);
  }

  return response;
}


HttpResponse RequestProcessor::buildAutoIndexResponse(RequestContext& ctx) {
  HttpResponse response;

  //This should always be true because we already check in the file resolver
  // if (ctx.getLocationConfig()->getAutoIndex()) {
    response.makeStatusLine();
    // std::string listing = "Index Listing should be sent here. Directory: " + ctx.getFilePath().string();

    std::string listing = generateAutoIndex(ctx.getFilePath(), ctx.getHttpRequest().getURI());
    response.addHeader("Content-Type", "text/html");
    response.addHeader("Content-Length", std::to_string(listing.size()));


    response.setBodySource(listing);
    return response;
  // }
  // return buildDefaultErrorResponse(403);
}

static std::string htmlEscape(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        switch (s[i]) {
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            case '"':  out += "&quot;"; break;
            case '\'': out += "&#39;";  break;
            default:   out += s[i];
        }
    }
    return out;
}

static std::string urlEncode(const std::string& s) {
    static const char* hex = "0123456789ABCDEF";
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = s[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            out += c;
        else {
            out += '%';
            out += hex[c >> 4];
            out += hex[c & 15];
        }
    }
    return out;
}

std::string RequestProcessor::generateAutoIndex(const std::string& dirPath, const std::string& uri) {
  std::vector<std::filesystem::directory_entry> entries;

  for (const auto& e : std::filesystem::directory_iterator(dirPath))
    entries.emplace_back(e);

  std::ostringstream html;
  html << "<!DOCTYPE html>\n<html>\n<head><title>Index of "
      << uri 
      << "</title></head>\n<body>\n<h1>Index of " << uri << "</h1>\n<hr>\n<ul>\n";

  for (const auto& e : entries) {
    std::string name = e.path().filename().string();
    std::string suffix = e.is_directory() ? "/" : "";
    html << "<li><a href= \""  << urlEncode(name) << suffix << "\">" << htmlEscape(name) << suffix << "</a></li> " << std::endl;
  }

  html << "</ul>\n<hr>\n</body>\n</html>\n";

  return html.str();
}


// Handles a delete request
// Assumes that the file path in the context is not a directory, since that case was handled before
// 
HttpResponse RequestProcessor::deleteHandler(RequestContext& ctx) {

  HttpResponse response;

  std::filesystem::path path = ctx.getFilePath();

  std::filesystem::path root;
  if (ctx.getLocationConfig())
    root = ctx.getLocationConfig()->getRoot();
  else
    root = ctx.getServerConfig()->getRoot();
  std::filesystem::path canonincalRoot = std::filesystem::canonical(root);
  std::filesystem::path canonicalTarget = std::filesystem::canonical(path);
  // Check that the resolved file is not the root
  if (std::filesystem::equivalent(canonicalTarget, canonincalRoot))
    return buildErrorResponse(403, ctx);
  
  std::error_code ec;

  bool removed = std::filesystem::remove(path, ec);

  if (ec) {
  if (ec == std::errc::permission_denied || ec == std::errc::operation_not_permitted ||
    ec == std::errc::read_only_file_system)
    return buildErrorResponse(403, ctx);
  if (ec == std::errc::no_such_file_or_directory)
    return buildErrorResponse(404, ctx);
  if (ec == std::errc::is_a_directory || ec == std::errc::directory_not_empty)
    return buildErrorResponse(409, ctx);
  return buildErrorResponse(500, ctx);
  }
  if (!removed)
    return buildErrorResponse(404, ctx);
  response.setStatusCode(204);
  response.makeStatusLine();
  return response;
}