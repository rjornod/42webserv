#pragma once

class HttpRequest;
class HttpResponse;
class ResolvedRoute;

class IRequestHandler {

  public:
    virtual ~IRequestHandler() {}
    virtual HttpResponse handle(const HttpRequest& request,
      const ResolvedRoute& route) = 0;

};