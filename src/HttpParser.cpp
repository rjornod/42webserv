#include "../include/HttpParser.hpp"
#include "../include/HttpMethod.hpp"

// HttpParser::~HttpParser() {}

HttpRequest HttpParser::parse() {

  HttpRequest request;

  parseRequestLine(request);
  splitHeadersBody();
  parseHeaders();
  request.setHeaders(m_headers);
  request.setBody(m_body);
  int contentLength = -1;
  if (m_headers.find("Content-Length") != m_headers.end())
    int contentLength = stoi(m_headers["Content-Lenght"]);
  std::cout << "Content-Length: " << contentLength << std::endl;
  return request;

}

void HttpParser::splitHeadersBody() {

  size_t firstCRLN = this->m_reqText.find("\r\n");
  size_t headerEnd = this->m_reqText.find("\r\n\r\n");
  this->m_headersString = this->m_reqText.substr(firstCRLN + 2, headerEnd);


  this->m_body = this->m_reqText.substr(headerEnd + 4);


}

std::string getHeaderName(std::string header) {
  size_t colon = header.find(':');

  if (colon == std::string::npos)
    return "";

  std::string name = header.substr(0, colon);

  return name;

}

std::string getHeaderValue(std::string header) {
  size_t colon = header.find(':');

  if (colon == std::string::npos)
    return "";

  std::string value = header.substr(colon + 1);

  return value;

}

void HttpParser::parseHeaders() {

  size_t start = 0;
  size_t end;

  while ((end = this->m_headersString.find("\r\n", start)) != std::string::npos) {
    std::string header = this->m_headersString.substr(start, end - start);
    if (header == "")
      break;
    this->m_headers.insert({getHeaderName(header), getHeaderValue(header)});
    start = end + 2;
  }

}

HttpMethod parseMethod(const std::string& method) {

  if (method == "GET")
    return HttpMethod::GET;
  if (method == "POST")
    return HttpMethod::POST;
  if (method == "DELETE")
    return HttpMethod::DELETE;
  else
    return HttpMethod::UNKNOWN;

}

bool HttpParser::parseRequestLine(HttpRequest& request){
  
  size_t firstCRLN = this->m_reqText.find("\r\n", 0);
  
  std::string firstLine = this->m_reqText.substr(0, firstCRLN);
  
  size_t firstSpace = firstLine.find(' ', 0);
  size_t secondSpace = firstLine.find(' ', firstSpace + 1);
  
  if (firstSpace == std::string::npos || secondSpace == std::string::npos)
  return false;
  
  std::string method = firstLine.substr(0, firstSpace);
  std::string uri = firstLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
  std::string version = firstLine.substr(secondSpace + 1);

  request.setMethod(parseMethod(method));
  request.setURI(uri);
  request.setVersion(version);
  
  return true;
}


// HttpMethod parseMethod(const std::string& method)
// {
//     static const std::unordered_map<std::string, HttpMethod> methods{
//         {"GET", HttpMethod::GET},
//         {"POST", HttpMethod::POST},
//         {"DELETE", HttpMethod::DELETE}
//     };

//     auto it = methods.find(method);
//     return (it != methods.end()) ? it->second : HttpMethod::UNKNOWN;
// }

// std::vector<std::string> HttpParser::split_lines(const std::string& text) {

//   std::vector<std::string> lines;

//   size_t start = 0;
//   size_t end;

//   while ((end = text.find("\r\n", start)) != std::string::npos) {
//     lines.push_back(text.substr(start, end - start));
//     start = end + 2;
//   }

//   if (start < text.size()) 
//     lines.push_back(text.substr(start));
  
//   return lines;
// }

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

// bool HttpParser::parseStartLine(HttpRequest& request) {

//   if (!this->m_headers.empty()) {
//     // std::vector<std::string> startLine = split(this->m_headers[0], ' ');
//     size_t firstSpace = this->m_headers[0].find(' ', 0);
//     size_t secondSpace = this->m_headers[0].find(' ', firstSpace + 1);

//     if (firstSpace == std::string::npos || secondSpace == std::string::npos)
//       return false;

//     std::string method = this->m_headers[0].substr(0, firstSpace);
//     std::string uri = this->m_headers[0].substr(firstSpace + 1, secondSpace - firstSpace - 1);
//     std::string version = this->m_headers[0].substr(secondSpace + 1);
//     request.setMethod(method);
//     request.setURI(uri);
//     request.setVersion(version);
//   }

//   return true;

// }