#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include "URI.hpp"
#include "HttpMethod.hpp"

class URI;

class HttpRequest {

  private:
    // std::string m_method;
    HttpMethod m_method;
    std::string m_requestLine;
    // URI& m_uri; //maybe not a pointer
    std::string m_rawPath; //URI for now
    // std::pair<int, int> m_version;//something for the version
    std::string m_version; //For now
    // std::vector<std::string> m_headers;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;//something else for the body
    
    

  public: 

    HttpRequest() : m_method(HttpMethod::UNKNOWN) {}
    HttpMethod getMethod() const {return m_method;}
    std::string getRequestLine() const {return m_requestLine;}
    std::string getURI() const {return m_rawPath;}
    std::string getVersion() const {return m_version;}
    std::string getBody() const {return m_body;}
    std::unordered_map<std::string, std::string> getHeaders() const {return m_headers;}
    void setMethod(HttpMethod method) {m_method = method;}
    void setRequestLine(std::string requestLine) {m_requestLine = requestLine;}
    void setURI(std::string rawPath) {m_rawPath = rawPath;}
    void setVersion(std::string version) {m_version = version;}
    void setHeaders(std::unordered_map<std::string, std::string> headers) {m_headers = headers;}
    void setBody(std::string body) {m_body = body;}

};

std::ostream &operator<<(std::ostream &out, const HttpRequest &request);