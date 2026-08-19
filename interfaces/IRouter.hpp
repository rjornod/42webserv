#pragma once

class HttpRequest;
class ResolvedRoute;
class GlobalConfig;
class Client;

class IRouter {

  public:

    virtual ~IRouter() {}
    virtual ResolvedRoute resolve(const HttpRequest& request,
      const GlobalConfig& config, int serverConfigIndex) = 0;
};