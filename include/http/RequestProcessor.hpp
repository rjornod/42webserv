#pragma once
#include "RequestContext.hpp"
#include "HttpResponse.hpp"
#include "Result.hpp"

class RequestProcessor {

  // private:

    // RequestContext m_requestContext;

  public:

    HttpResponse process(RequestContext& ctx);
    bool allowedMethod(const RequestContext& ctx);
    HttpResponse staticHandler(RequestContext& ctx);
    HttpResponse buildErrorResponse(int errorCode);

};