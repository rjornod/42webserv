#pragma once

#include <string>
#include <vector>
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
    std::vector<std::string> split_lines(const std::string& text);
    bool parseStartLine(HttpRequest& request);

  private:

    std::string m_reqText;
    std::string m_headersString;
    std::string m_body;
    std::vector<std::string> m_startLine;
    std::vector<std::string> m_headers;

};