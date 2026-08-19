#pragma once
#include "HttpRequest.hpp"
#include "ResolvedRoute.hpp"
#include "GlobalConfig.hpp"
#include "IRouter.hpp"
#include "Client.hpp"

class Router : public IRouter {

  public:

    ~Router() {}

    ResolvedRoute resolve(const HttpRequest& request, const GlobalConfig& config,
      int serverConfigIndex);
    LocationConfig matchLocation(std::vector<LocationConfig> locationConfigs, std::string uri);
};