#include "RequestProcessor.hpp"
#include "FileResolver.hpp"
#include <algorithm>

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
  Result<std::filesystem::path, FileResolutionError> path = fileResolver.resolve(ctx);
  if (!path) {
    switch (path.error()) {
      case FileResolutionError::BAD_REQUEST:
        return buildErrorResponse(400);
        // break;
      case FileResolutionError::FORBIDDEN:
        return buildErrorResponse(403);
        // break;
      case FileResolutionError::NOT_FOUND:
        return buildErrorResponse(404);
        break;
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
  ctx.setFilePath(path.value());

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
  response.setBodySource(path);

  return response;
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
  response.addHeader("Content-Type", "text/plain");
  response.addHeader("Content-Length", std::to_string(reason.size()));
  response.setBodySource(std::to_string(errorCode) + " " + reason);

  return response;
}