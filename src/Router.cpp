#include "Router.hpp"


/*
Function that does the routing, i.e. searches in the globalConfig, for the relevant serverconfig,
and then does max prefix matching to find the relevant location

NOTE: Instead of passing the global config and the index, why not pass the relevant server config right away
if it's unique... (?!)
*/
RequestContext Router::createContext(const HttpRequest& request, const GlobalConfig& config, int serverConfigIndex){
  
  const ServerConfig& serverConfig = config.getServerConfigs()[serverConfigIndex];
  auto uri = request.getURI();
  RequestContext ctx;
  ctx.setServerConfig(serverConfig);
  ctx.setHttpRequest(request);
  ctx.setLocationConfig(matchLocation(serverConfig.getLocationConfigs(), uri));

  return ctx;

}

/* Checks whether the prefix is actually a "complete" prefix

For example for uri /images-old/logo.png  the path /images-old is a "well formed"
prefix but /images shouldn't be
*/
bool Router::isPrefixMatch(std::string_view path, std::string uri) {
  if (uri.std::string::starts_with(path)) {
    std::string_view uri_view = uri;
    uri_view.remove_prefix(path.size());
    if (uri_view.empty() || *uri_view.begin() == '/')
      return true;
  }
  return false;
}

// What if there's no matching location?!
const LocationConfig* Router::matchLocation(const std::vector<LocationConfig>& locationConfigs, std::string uri) {
  
  size_t maxLen = 0;
  const LocationConfig* matchingLocation = nullptr;

  for (const LocationConfig& location : locationConfigs) {
    std::string path = location.getPath();
    if (path == "/" || isPrefixMatch(path, uri)) {
      if (path.size() > maxLen) {
        maxLen = path.size();
        matchingLocation = &location; // Not sure if this is too costly
        // std::cout << "Path: " << path << " is prefix of length "<< maxLen << std::endl;
      }
    }
    // else
    //   std::cout << "Path: " << path << "is NOT prefix" << std::endl;
  }
  return matchingLocation;
}


// bool Router::mapEffectiveRoot(std::string_view root, std::string_view uri) {
//   return false;
// }