#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include "URI.hpp"
#include "HttpMethod.hpp"
#include "HttpVersion.hpp"

class URI;

class HttpRequest {

  private:
    HttpMethod m_method;
    std::string m_rawPath;
    HttpVersion m_version; 
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;
    
    

  public: 

    HttpRequest() : m_method(HttpMethod::UNKNOWN) {}
    HttpMethod getMethod() const {return m_method;}
    const std::string& getURI() const {return m_rawPath;}
    HttpVersion getVersion() const {return m_version;}
    std::string getBody() const {return m_body;}
    std::unordered_map<std::string, std::string> getHeaders() const {return m_headers;}
    void setMethod(HttpMethod method) {m_method = method;}
    void setURI(std::string_view rawPath) {m_rawPath = rawPath;}
    void setVersion(HttpVersion version) {m_version = version;}
    void setHeaders(std::unordered_map<std::string, std::string> headers) {m_headers = headers;}
    void setBody(std::string body) {m_body = body;}

};

std::ostream &operator<<(std::ostream &out, const HttpRequest &request);