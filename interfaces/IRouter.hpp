#pragma once

class HttpRequest;
class RequestContext;
class GlobalConfig;
class Client;

class IRouter {

  public:

    virtual ~IRouter() {}
    virtual RequestContext createContext(const HttpRequest& request, const ServerConfig& serverConfig) = 0;
};