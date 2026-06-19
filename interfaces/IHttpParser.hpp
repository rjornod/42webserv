#pragma once

#include "../include/HttpRequest.hpp"

class IHttpParser {

  public:

    virtual ~IHttpParser() {}

    
    virtual HttpRequest parse() = 0;

};