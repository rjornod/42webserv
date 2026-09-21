#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include "HttpRequest.hpp"
#include "IHttpParser.hpp"
#include "HttpParserState.hpp"
#include "HttpVersion.hpp"

class HttpParser : public IHttpParser {

  private:

    std::string m_buffer;
    std::string m_body;
    std::unordered_map<std::string, std::string> m_headers;
    long m_expectedBodyLen;
    HttpParserState m_state;
    HttpRequest m_request;
    std::string m_errorMessage;

  public:

    // TO DO -- check if you actually need to initialize expected body len
    HttpParser() : m_expectedBodyLen(-1), m_state(HttpParserState::REQUEST_LINE) {}
    ~HttpParser() {}

    std::unordered_map<std::string, std::string> getHeaders() const {return m_headers;}
    std::string getBody() const {return m_body;}
    std::string getBuffer() const {return m_buffer;}
    int getExpectedBodyLen() const {return m_expectedBodyLen;}
    HttpRequest getRequest() const {return m_request;}
    HttpParserState getParserState() const {return m_state;}
    std::string getErrorMessage() const {return m_errorMessage;}
    void setParserState(HttpParserState state) {m_state = state;}

    void parse(std::string_view chunk);
    bool parseRequestLine();
    bool parseHeaders();
    bool parseBody();
    void determineBodyLength();
    void reportErrors();

    //Mainly for the tests, clear the parser in order to reuse it
    void clearParser();

    HttpVersion validateHttpVersion(std::string_view versionStr);
    // HttpVersion  parseHttpVersion(std::string_view version);

    //DEBUG
    void printHeaders();
};