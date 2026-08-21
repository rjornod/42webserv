#include "RequestProcessor.hpp"
#include "FileResolver.hpp"

HttpResponse RequestProcessor::process(RequestContext& ctx){

  HttpResponse response(404);
  FileResolver fileResolver;

  ctx.setFilePath(fileResolver.resolve(ctx));

  // Check method, ofr example
  // if (!methodAllowed(ctx))
    // return HttpResponse::methodNotAllowed();

  // Call appropriate handler
  // return staticFileHandler.handle(ctx);

  std::cout << "FilePath: " << ctx.getFilePath() << std::endl;

  return response;
}