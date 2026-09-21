#pragma once
#include "HttpRequest.hpp"
#include "RequestContext.hpp"
#include "GlobalConfig.hpp"
#include "IRouter.hpp"
#include "Client.hpp"

class Router : public IRouter {

  public:

    ~Router() {}

    RequestContext   createContext(const HttpRequest& request, const GlobalConfig& config,
      int serverConfigIndex);
    const LocationConfig*  matchLocation(const std::vector<LocationConfig>& locationConfigs, std::string uri);
    bool isPrefixMatch(std::string_view path, std::string uri);
};