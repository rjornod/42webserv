#pragma once

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "RequestContext.hpp"

class RequestHandler {

  public:

    ~RequestHandler() {}

    HttpResponse handle(const HttpRequest& request, const RequestContext& ctx);

};