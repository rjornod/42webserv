#include "FileResolver.hpp"



std::string FileResolver::resolve(const RequestContext& ctx){

  std::string locationRoot = ctx.getLocationConfig()->getRoot();

  std::string rootMinusSlash = locationRoot.erase(0, 1);

  std::string filePath = rootMinusSlash + ctx.getHttpRequest().getURI();

  return filePath;
}