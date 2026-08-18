#pragma once

#include <string>
#include <map>

class HttpResponse {

  private:
    int m_statusCode;
    static const std::map<int, std::string> M_STATUSCODES;


};