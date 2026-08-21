#pragma once
#include "RequestContext.hpp"
#include "HttpResponse.hpp"

class RequestProcessor {

  private:

    // RequestContext m_requestContext;

  public:

    HttpResponse process(RequestContext& ctx);

};