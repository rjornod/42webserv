#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include "HttpRequest.hpp"
#include "../interfaces/IHttpParser.hpp"
#include "HttpParserState.hpp"

class HttpParser : public IHttpParser {

  private:

    std::string m_buffer;
    std::string m_body;
    std::unordered_map<std::string, std::string> m_headers;
    int m_expectedBodyLen;
    HttpParserState m_state;
    HttpRequest m_request;

  public:

    // TO DO -- check if you actually need to initialize expected body len
    HttpParser() : m_expectedBodyLen(-1), m_state(HttpParserState::REQUEST_LINE) {}
    ~HttpParser() {}

    // std::string getHeadersString() {return m_headersString;}
    std::unordered_map<std::string, std::string> getHeaders() const {return m_headers;}
    std::string getBody() const {return m_body;}
    std::string getBuffer() const {return m_buffer;}
    int getExpectedBodyLen() const {return m_expectedBodyLen;}
    HttpRequest getRequest() const {return m_request;}
    HttpParserState getParserState() const {return m_state;}
    void setParserState(HttpParserState state) {m_state = state;}

    //New version with switch --------
    void partialParse(const std::string& chunk);
    bool parseRequestLine();
    bool parseHeaders();
    bool parseBody();
    void determineBodyLength();
    void buildRequest();

    //DEBUG
    void printHeaders();
    
    
    // // Old version -- with bools
    // bool partialParse(const std::string& reqString);
    // bool parseReqLine(HttpRequest& request, const std::string& reqString);
    // void parsePartHeaders(int start, const std::string& reqString);
    
    
    
    // HttpRequest parse();
    // void splitHeadersBody();
    // void parseHeaders();
    // bool parseRequestLine(HttpRequest& request);
    // std::vector<std::string> split_lines(const std::string& text);
    // bool parseStartLine(HttpRequest& request);
};