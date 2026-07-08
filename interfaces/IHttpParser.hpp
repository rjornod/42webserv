#pragma once

#include "../include/HttpRequest.hpp"

class IHttpParser {

  public:

    virtual ~IHttpParser() {}

    
    virtual void partialParse(const std::string& chunk) = 0;

};