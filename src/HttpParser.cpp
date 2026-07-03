#include "../include/HttpParser.hpp"
#include "../include/HttpMethod.hpp"


//Should this function always return an HttpRequest, or should it just store
// it and when the parserState == complete, then we can retrieve it?
HttpRequest HttpParser::parse() {

  HttpRequest request;

  parseRequestLine(request);
  splitHeadersBody();
  parseHeaders();
  request.setHeaders(m_headers);
  request.setBody(m_body);
  int contentLength = -1;
  auto it = m_headers.find("Content-Length");
  if (it != m_headers.end())
    contentLength = std::stoi(it->second);
  std::cout << "Content-Length: " << contentLength << std::endl;
  return request;

}

void HttpParser::buildRequest() {
  m_request.setHeaders(m_headers);
  m_request.setBody(m_body);
  
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
  
  while ((end = m_headersString.find("\r\n", start)) != std::string::npos) {
    std::string header = m_headersString.substr(start, end - start);
    if (header == "")
    break;
    m_headers.insert({getHeaderName(header), getHeaderValue(header)});
    start = end + 2;
  }
  
}

void HttpParser::parsePartHeaders(int start, const std::string& reqString) {
  
  
  size_t end;
  
  while ((end = reqString.find("\r\n", start)) != std::string::npos) {
    std::string header = reqString.substr(start, end - start);
    if (header == "")
      break;
    m_headers.insert({getHeaderName(header), getHeaderValue(header)});
    start = end + 2;
  }
}

bool HttpParser::partialParse(const std::string& reqString) {

  size_t start = 0;

  if (!m_reqLine) {
    size_t firstCRLN = reqString.find("\r\n", 0);
    if (firstCRLN == std::string::npos) {
      m_state = HttpParserState::REQUEST_LINE;
      // parsePartReqLine(reqString);
      return true;
    }
    parseReqLine(m_request, reqString);
    m_reqLine = true;
    start = firstCRLN + 2;
  }

  size_t headerEnd = reqString.find("\r\n\r\n");

  if (headerEnd == std::string::npos) {
    m_state = HttpParserState::HEADERS;
    parsePartHeaders(start, reqString); //New parseHeaders that 
    return true;
  }

  parsePartHeaders(start, reqString);

  // TODO: First check if we already knew the content length
  int contentLen = -1;
  auto it = m_headers.find("Content-Length");
  if (it == m_headers.end()) {
    m_body = "";
    m_state = HttpParserState::COMPLETE;
    //Still need to build the request
    return true;
  }

  contentLen = std::stoi(it->second);
  m_expected = contentLen;
  std::string partialBody = reqString.substr(headerEnd + 4);
  int available = partialBody.length();
  if (available < m_expected)
    m_state = HttpParserState::INCOMPLETE;
  if (available == m_expected)
    m_state = HttpParserState::COMPLETE;
  m_body.append(partialBody);
  // if (available > m_expected)
    //Next request starts... idk for now
  if (m_state == HttpParserState::COMPLETE)
    buildRequest();
  return true;
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

//Assuming there is a full request line passed
bool HttpParser::parseReqLine(HttpRequest& request, const std::string& reqString) {
  
  size_t firstCRLN = reqString.find("\r\n", 0);
  
  std::string firstLine = reqString.substr(0, firstCRLN);

  std::cout << "[DEBUG] Req Line: " << firstLine << std::endl;
  
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

void HttpParser::printHeaders() {
  for (auto it = m_headers.begin(); it != m_headers.end(); it++) {
    std::cout << it->first << ": " << it->second << std::endl;
  }
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