#include "FileResolver.hpp"

std::string FileResolver::resolve(const RequestContext& ctx){

  std::string filePath = ctx.getLocationConfig().getRoot() + ctx.getHttpRequest().getURI();

  return filePath;
}