#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "HttpRequest.hpp"
#include "../interfaces/IHttpParser.hpp"

class HttpParser : public IHttpParser {

  public:

    HttpParser(std::string reqText) : m_reqText(reqText) {}
    ~HttpParser();

    std::string getHeaders() {return m_headersString;}
    std::string getBody() {return m_body;}

    HttpRequest parse();
    void splitHeadersBody();
    void parseHeaders();
    bool parseRequestLine(HttpRequest& request);
    // std::vector<std::string> split_lines(const std::string& text);
    // bool parseStartLine(HttpRequest& request);

  private:

    std::string m_reqText;
    std::string m_headersString;
    std::string m_body;
    std::vector<std::string> m_startLine;
    std::unordered_map<std::string, std::string> m_headers;

};