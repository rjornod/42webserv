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

  if (!allowedMethod(ctx)) {
    return buildErrorResponse(405, ctx);
  }

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
HttpResponse RequestProcessor::handleDelete(RequestContext& ctx) {

  Resolution res;
  HttpResponse err;

  if (!resolvePath(ctx, res, err))
    return err;
  if (res.kind == Resolution::DIRECTORY)
    return buildErrorResponse(409, ctx);
  return deleteHandler(ctx);
}

HttpResponse RequestProcessor::handlePost(RequestContext& ctx) {
  const HttpRequest& req = ctx.getHttpRequest();

  if (!postHeadersPresent(req))
    return buildErrorResponse(411, ctx);
  if (!checkContentLength(ctx))
    return buildErrorResponse(413, ctx);
  std::cout << "Handling POST request" << std::endl;
  return HttpResponse();
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


// HttpResponse RequestProcessor::process(RequestContext& ctx){

//   HttpResponse response;
//   FileResolver fileResolver;

//   // Check method
//   if (!allowedMethod(ctx)) {
//     return buildDefaultErrorResponse(405);
//   }

//   HttpMethod method = ctx.getHttpRequest().getMethod();


//   Result<Resolution, FileResolutionError> pathResult = fileResolver.resolve(ctx);

  
//   if (!pathResult) {
//     switch (pathResult.error()) {
//       case FileResolutionError::BAD_REQUEST:
//         return buildErrorResponse(400, ctx);
//       case FileResolutionError::FORBIDDEN:
//         return buildErrorResponse(403, ctx);
//       case FileResolutionError::NOT_FOUND:
//         return buildErrorResponse(404, ctx);
//       case FileResolutionError::SERVER_ERROR:
//         return buildErrorResponse(500, ctx);
//     }
//   }
  
//   // The file path is canonical
//   ctx.setFilePath(pathResult.value().path);

  
//   HttpRequest req = ctx.getHttpRequest();

//   switch (method) {
//     case HttpMethod::GET:
//       if (pathResult.value().kind == Resolution::DIRECTORY)
//         return buildAutoIndexResponse(ctx);
//       response = staticHandler(ctx);
//       break;
//     case HttpMethod::POST:
//       std::cout << "Handling post" << std::endl;
//       if (!postHeadersPresent(req))
//         return buildErrorResponse(411, ctx);
//       if (!checkContentLength(ctx))
//         return buildErrorResponse(413, ctx);
//       break;
//     case HttpMethod::DELETE:
//       std::cout << "Handling delete" << std::endl;
//       if (pathResult.value().kind == Resolution::DIRECTORY) 
//         return buildErrorResponse(409, ctx);
//       response = deleteHandler(ctx);
//       break;
//     default:
//       std::cout << "Handling method not allowed" << std::endl;
//   }

//   return response;
// }

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
  return (contentLen != headers.end() || transferEncoding != headers.end());
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
  // std::cout << "Called the static handler on context with filepath:" << ctx.getFilePath() << std::endl;
  
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
  {"pdf", "application/pdf"}
};

Result<std::string, MimeTypeError> RequestProcessor::guessMimeType(std::string file) {

  std::string fileName = file;

  size_t dot = fileName.find(".");

  if (dot == std::string::npos)
    return Result<std::string, MimeTypeError>::Ok("application/octet-stream");

  std::string ext = fileName.substr(dot + 1);

  // std::cout << "File extension: " << ext << std::endl;

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
  // std::cout << "BUILDING ERROR RESPONSE" << std::endl;
  if (auto errorPage = findErrorPage(errorCode, ctx)) {
    // std::cout << "FOUND ERROR PAGE" << std::endl;
    std::cout << *errorPage << std::endl;
    RequestContext errorCtx = makeErrorCtx(ctx, *errorPage);
    // std::cout << "Error context built: " << std::endl;
    // errorCtx.getLocationConfig()->printValues();
    // std::cout << "File path from error ctx" << errorCtx.getFilePath();
    FileResolver fileResolver;
    auto result = fileResolver.resolve(errorCtx);
    if (!result)
      std::cout << "error on resolving error page path " << to_string(result.error()) << std::endl;
    if (result && result.value().kind == Resolution::FILE) {
      // std::cout << "Will serve " <<  result.value().path << std::endl;
      errorCtx.setFilePath(result.value().path);
      HttpResponse response = staticHandler(errorCtx);
      response.updateStatus(errorCode);
      return response;
      // return buildDefaultErrorResponse(errorCode); // For debugging
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