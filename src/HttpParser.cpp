#include "../include/HttpParser.hpp"

HttpParser::~HttpParser() {}

HttpRequest HttpParser::parse() {

  HttpRequest request;

  this->splitHeadersBody();
  this->parseStartLine(request);

  // request.setHeaders(this->m_headers)
  request.setBody(this->m_body);

  return request;

}

void HttpParser::splitHeadersBody() {

  size_t headerEnd = this->m_reqText.find("\r\n\r\n");
  this->m_headersString = this->m_reqText.substr(0, headerEnd);
  this->m_body = this->m_reqText.substr(headerEnd + 4);

  this->m_headers = this->split_lines(this->m_headersString);

}

std::vector<std::string> HttpParser::split_lines(const std::string& text) {

  std::vector<std::string> lines;

  size_t start = 0;
  size_t end;

  while ((end = text.find("\r\n", start)) != std::string::npos) {
    lines.push_back(text.substr(start, end - start));
    start = end + 2;
  }

  if (start < text.size()) 
    lines.push_back(text.substr(start));
  
  return lines;
}

// std::vector<std::string> split(const std::string& text, char delimiter) {

//   std::vector<std::string> lines;

//   size_t start = 0;
//   size_t end;

//   while ((end = text.find(delimiter, start)) != std::string::npos) {
//     lines.push_back(text.substr(start, end - start));
//     start = end + 2;
//   }

//   if (start < text.size()) 
//     lines.push_back(text.substr(start));
  
//   return lines;
// }

bool HttpParser::parseStartLine(HttpRequest& request) {

  if (!this->m_headers.empty()) {
    // std::vector<std::string> startLine = split(this->m_headers[0], ' ');
    size_t firstSpace = this->m_headers[0].find(' ', 0);
    size_t secondSpace = this->m_headers[0].find(' ', firstSpace + 1);

    if (firstSpace == std::string::npos || secondSpace == std::string::npos)
      return false;

    std::string method = this->m_headers[0].substr(0, firstSpace);
    std::string uri = this->m_headers[0].substr(firstSpace + 1, secondSpace - firstSpace - 1);
    std::string version = this->m_headers[0].substr(secondSpace + 1);
    request.setMethod(method);
    request.setURI(uri);
    request.setVersion(version);
  }

  return true;

}