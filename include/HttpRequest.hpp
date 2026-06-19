#pragma once

#include <string>
#include <iostream>
#include <vector>
#include "URI.hpp"

class URI;

class HttpRequest {

  private:
    std::string m_method;
    // URI& m_uri; //maybe not a pointer
    std::string m_rawPath; //URI for now
    // std::pair<int, int> m_version;//something for the version
    std::string m_version; //For now
    std::vector<std::string> m_headers;
    std::string m_body;//something else for the body
    
    

  public: 

    std::string getMethod() const {return m_method;}
    std::string getURI() const {return m_rawPath;}
    std::string getVersion() const {return m_version;}
    std::string getBody() const {return m_body;}
    void setMethod(std::string method) {m_method = method;}
    void setURI(std::string rawPath) {m_rawPath = rawPath;}
    void setVersion(std::string version) {m_version = version;}
    void setHeaders(std::vector<std::string> headers) {m_headers = headers;}
    void setBody(std::string body) {m_body = body;}

};

std::ostream &operator<<(std::ostream &out, const HttpRequest &request);