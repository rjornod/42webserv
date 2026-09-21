#pragma once

class HttpRequest;
class HttpResponse;
class RequestContext;

class IRequestHandler {

  public:
    virtual ~IRequestHandler() {}
    virtual HttpResponse handle(const HttpRequest& request,
      const RequestContext& route) = 0;

};