#pragma once

#include <string>
#include <map>

class HttpResponse {

  private:
    int m_statusCode;
    // std::string m_body; // For now it's a string
    static const std::map<int, std::string> M_STATUSCODES;

  public:

    HttpResponse(int statusCode) : m_statusCode(statusCode) {}

    int getStatusCode() const {return m_statusCode;}
    void setStatusCode(int status) {m_statusCode = status;}
    // std::string getStatusMessage() {return M_STATUSCODES[m_statusCode];}


};