#include "FileResolver.hpp"



std::string FileResolver::resolve(const RequestContext& ctx){

  std::string filePath = (ctx.getLocationConfig()->getRoot().erase(0, 1)) + ctx.getHttpRequest().getURI();

  return filePath;
}