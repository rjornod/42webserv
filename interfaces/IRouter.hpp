#pragma once

class HttpRequest;
class RequestContext;
class GlobalConfig;
class Client;

class IRouter {

  public:

    virtual ~IRouter() {}
    virtual RequestContext resolve(const HttpRequest& request,
      const GlobalConfig& config, int serverConfigIndex) = 0;
};