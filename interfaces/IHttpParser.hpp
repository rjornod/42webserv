#pragma once

#include "../include/HttpRequest.hpp"

class IHttpParser {

  public:

    virtual ~IHttpParser() {}

    
    virtual void parse(std::string_view chunk) = 0;

};