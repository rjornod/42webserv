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
  ctx.setFilePath(fileResolver.resolve(ctx));

  // Check method
  if (!allowedMethod(ctx)) {
    response.setStatusCode(405);
  }


  // Call appropriate handler
  // return staticFileHandler.handle(ctx);

  // std::cout << "FilePath: " << ctx.getFilePath() << std::endl;/

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