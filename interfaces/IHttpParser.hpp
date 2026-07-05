#pragma once

#include "../include/HttpRequest.hpp"

class IHttpParser {

  public:

    virtual ~IHttpParser() {}

    
    virtual bool partialParse(const std::string& reqString) = 0;

};