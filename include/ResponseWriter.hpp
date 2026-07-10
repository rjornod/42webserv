#pragma once

#include "HttpResponse.hpp"

class ResponseWriter {

  public:
    size_t write_to_buffer() {}

  private:

    HttpResponse m_response;
    std::string m_buffer;
};