#include "RequestProcessor.hpp"
#include "FileResolver.hpp"
#include <algorithm>
#include <sstream>

HttpResponse RequestProcessor::process(RequestContext& ctx){

  HttpResponse response;
  FileResolver fileResolver;

  if (ctx.getLocationConfig() == nullptr) {
    response.setStatusCode(500);              //Actually it should just take the root from the server
    return response;
  }

  // Check method
  if (!allowedMethod(ctx)) {
    return buildErrorResponse(405);
  }

  // std::cout << "File path: " << fileResolver.resolve(ctx) << std::endl;
  // ctx.setFilePath(fileResolver.resolve(ctx));
  Result<Resolution, FileResolutionError> pathResult = fileResolver.resolve(ctx);
  if (!pathResult) {
    switch (pathResult.error()) {
      case FileResolutionError::BAD_REQUEST:
        return buildErrorResponse(400);
        // break;
      case FileResolutionError::FORBIDDEN:
        return buildErrorResponse(403);
        // break;
      case FileResolutionError::NOT_FOUND:
        return buildErrorResponse(404);
        // break;
      case FileResolutionError::SERVER_ERROR:
        return buildErrorResponse(500);
        // break;
      default:
        return buildErrorResponse(418);; //Not sure what to do in this case
        // break;
    }
    return response;
  }
  // The file path is canonical
  ctx.setFilePath(pathResult.value().path);
  if (pathResult.value().kind == Resolution::DIRECTORY)
    return buildAutoIndexResponse(ctx);

  HttpMethod method = ctx.getHttpRequest().getMethod();

  switch (method) {
    case HttpMethod::GET:
      response = staticHandler(ctx);
      break;
    case HttpMethod::POST:
      std::cout << "Handling upload" << std::endl;
      break;
    case HttpMethod::DELETE:
      std::cout << "Handling delete" << std::endl;
      break;
    default:
      std::cout << "Handling method not allowed" << std::endl;
  }


  // Call appropriate handler that will build the appropriate response
  // return staticFileHandler.handle(ctx);

  // std::cout << "FilePath: " << ctx.getFilePathName() << std::endl;

  return response;
}

bool RequestProcessor::allowedMethod(const RequestContext& ctx) {
  HttpMethod requestMethod = ctx.getHttpRequest().getMethod();

  std::vector<std::string> allowedMethods = ctx.getLocationConfig()->getAllowedMethods();

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
  response.setStatusCode(200);
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

HttpResponse RequestProcessor::buildErrorResponse(int errorCode) {

  HttpResponse response;

  switch(errorCode) {
    case 400:
      response.setStatusCode(400);
      break;
    case 403:
      response.setStatusCode(403);
      break;
    case 404:
      response.setStatusCode(404);
      break;
    case 405:
      response.setStatusCode(405);
      break;
    case 500:
      response.setStatusCode(500);
        break;
    default:
      response.setStatusCode(418); // Not sure what to do in this case -- I guess we would never call the function with another status code
      break;
  }

  response.makeStatusLine();
  std::string reason = response.getReasonPhrase();
  response.addHeader("Content-Type", "text/html");
  response.addHeader("Content-Length", std::to_string((std::to_string(errorCode) + " " + reason).size()));
  response.setBodySource(std::to_string(errorCode) + " " + reason);

  return response;
}


// TO DO: Consider moving this responsibility back to the FileResolver
HttpResponse RequestProcessor::buildAutoIndexResponse(RequestContext& ctx) {
  HttpResponse response;

  //This should always be true because we already check in the file resolver
  if (ctx.getLocationConfig()->getAutoIndex()) {
    response.makeStatusLine();
    // std::string listing = "Index Listing should be sent here. Directory: " + ctx.getFilePath().string();

    std::string listing = generateAutoIndex(ctx.getFilePath(), ctx.getHttpRequest().getURI());
    response.addHeader("Content-Type", "text/html");
    response.addHeader("Content-Length", std::to_string(listing.size()));


    response.setBodySource(listing);
    return response;
  }
  return buildErrorResponse(403);
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