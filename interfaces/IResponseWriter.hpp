#pragma once

#include "HttpResponse.hpp"

enum class WriteResult;

class IResponseWriter {
  
  public:
    
    WriteResult writeTo(int socketFd);
    virtual ~IResponseWriter() {}

};