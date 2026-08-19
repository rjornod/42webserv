#include "Router.hpp"


/*
Function that does the routing, i.e. searches in the globalConfig, for the relevant serverconfig,
and then does max prefix matching to find the relevant location

NOTE: Instead of passing the global config and the index, why not pass the relevant server config right away
if it's unique... (?!)
*/
ResolvedRoute Router::resolve(const HttpRequest& request, const GlobalConfig& config, int serverConfigIndex){
  ServerConfig serverConfig = config.getServerConfigs()[serverConfigIndex];

  std::string uri = request.getURI();

  // std::cout << "URI: " << uri << std::endl;

  // std::cout << "Relevant locations: " << uri << std::endl;
  // for (const LocationConfig& location : serverConfig.getLocationConfigs()) {
  //   location.printValues();
  // }

  ResolvedRoute resolvedRoute;
  resolvedRoute.setLocationConfig(matchLocation(serverConfig.getLocationConfigs(), uri));

  return resolvedRoute;

}

LocationConfig Router::matchLocation(std::vector<LocationConfig> locationConfigs, std::string uri) {
  
  size_t maxLen = 0;
  LocationConfig matchingLocation;

  std::cout << "URI: " << uri << std::endl;

  for (LocationConfig& location : locationConfigs) {
    std::string_view path = location.getPath();
    if (uri.std::string::starts_with(path)) {
      if ((path.size() > maxLen)) {
        maxLen = path.size();
        matchingLocation = location; // Not sure if this is too costly
        // std::cout << "Path: " << path << " is prefix of length "<< maxLen << std::endl;
      }
    }
    // else
      // std::cout << "Path: " << path << "is NOT prefix" << std::endl;
  }
  return matchingLocation;
}