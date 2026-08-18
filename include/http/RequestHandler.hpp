#pragma once
#include "IRequestHandler.hpp"

class RequestHandler : public IRequestHandler {

  public:

    ~RequestHandler() {}

    HttpResponse handle(const HttpRequest& request, const ResolvedRoute& route);

};