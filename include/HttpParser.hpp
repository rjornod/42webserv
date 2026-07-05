#pragma once

#include <string>
#include <vector>
// #include <algorithm>
#include <unordered_map>
#include <iostream>
#include "HttpRequest.hpp"
#include "../interfaces/IHttpParser.hpp"
#include "HttpParserState.hpp"

class HttpParser : public IHttpParser {

  private:

    // std::string m_reqText;
    // std::string m_headersString;
    // std::vector<std::string> m_startLine;
    std::string m_buffer;
    std::string m_body;
    std::unordered_map<std::string, std::string> m_headers;
    bool m_reqLine;
    int m_expected;
    int m_available;
    // int m_contentLen;
    HttpParserState m_state;
    HttpRequest m_request;

  public:

    HttpParser() : m_reqLine(false), m_expected(-1) {}
    // HttpParser(std::string reqText) : m_reqText(reqText) {}
    ~HttpParser() {}

    // std::string getHeadersString() {return m_headersString;}
    std::unordered_map<std::string, std::string> getHeaders() const {return m_headers;}
    std::string getBody() {return m_body;}
    int getExpected() const {return m_expected;}
    int getAvailable() const {return m_available;}
    HttpRequest getRequest() const {return m_request;}
    HttpParserState getParserState() const {return m_state;}

    void setParserState(HttpParserState state) {m_state = state;}

    // HttpRequest parse();
    bool partialParse(const std::string& reqString);
    // void splitHeadersBody();
    void parsePartHeaders(int start, const std::string& reqString);
    bool parseReqLine(HttpRequest& request, const std::string& reqString);
    void buildRequest();
    void printHeaders();
    // void parseHeaders();
    // bool parseRequestLine(HttpRequest& request);
    // std::vector<std::string> split_lines(const std::string& text);
    // bool parseStartLine(HttpRequest& request);
};