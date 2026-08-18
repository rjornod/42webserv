#pragma once

class HttpRequest;
class ResolvedRoute;
class GlobalConfig;

class IRouter {

  public:

    virtual ~IRouter() {}
    virtual ResolvedRoute resolve(const HttpRequest& request,
      const GlobalConfig& config) = 0;
};