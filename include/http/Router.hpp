#pragma once
#include "HttpRequest.hpp"
#include "RequestContext.hpp"
#include "GlobalConfig.hpp"
#include "Client.hpp"

class Router {

  public:

    ~Router() {}

    RequestContext   createContext(const HttpRequest& request, const ServerConfig& serverConfig);
    RequestContext createErrorContext(const ServerConfig& serverConfig);
    const LocationConfig*  matchLocation(const std::vector<LocationConfig>& locationConfigs, std::string uri);
    bool isPrefixMatch(std::string_view path, std::string uri);
};