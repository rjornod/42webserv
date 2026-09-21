#include "RequestProcessor.hpp"
#include "FileResolver.hpp"
#include <algorithm>

HttpResponse RequestProcessor::process(RequestContext& ctx){

  HttpResponse response(200);
  FileResolver fileResolver;

  if (ctx.getLocationConfig() == nullptr) {
    response.setStatusCode(500);              //Actually it should just take the root from the server
    return response;
  }

  // std::cout << "File path: " << fileResolver.resolve(ctx) << std::endl;
  // ctx.setFilePath(fileResolver.resolve(ctx));
  Result<std::filesystem::path, FileResolutionError> path = fileResolver.resolve(ctx);
  if (!path) {
    switch (path.error()) {
      case FileResolutionError::BAD_REQUEST:
        response.setStatusCode(400);
        break;
      case FileResolutionError::FORBIDDEN:
        response.setStatusCode(403);
        break;
      case FileResolutionError::NOT_FOUND:
        response.setStatusCode(404);
        break;
      case FileResolutionError::SERVER_ERROR:
        response.setStatusCode(500);
        break;
      default:
        response.setStatusCode(418); //Not sure what to do in this case
        break;
    }
    return response;
  }
  // The file path is canonical
  ctx.setFilePath(path.value());

  // Check method
  if (!allowedMethod(ctx)) {
    response.setStatusCode(405);
  }


  // Call appropriate handler that will build the appropriate response
  // return staticFileHandler.handle(ctx);

  std::cout << "FilePath: " << ctx.getFilePathName() << std::endl;

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