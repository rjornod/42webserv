#include "../include/HttpParser.hpp"
#include "../include/HttpMethod.hpp"

void HttpParser::partialParse(const std::string& chunk) {

  m_buffer += chunk;

  switch (getParserState()) {
    case HttpParserState::REQUEST_LINE:
      if (!parseRequestLine())
        return;

      m_state = HttpParserState::HEADERS;

    case HttpParserState::HEADERS:
      if (!parseHeaders())
        return;
      determineBodyLength();
      if (m_expectedBodyLen == 0) {
        m_request.setBody("");
        m_state = HttpParserState::COMPLETE;
        // buildRequest(); // Not sure
        return;
      }
      m_state = HttpParserState::BODY;
    
    case HttpParserState::BODY:
      if (!parseBody())
        return;
      m_state = HttpParserState::COMPLETE;

    case HttpParserState::COMPLETE:

    case HttpParserState::ERROR:
      std::cout << "Error on parsing" << std::endl;
      
    default:
      break;
  }
  
}

void HttpParser::buildRequest() {

  m_request.setHeaders(m_headers);
  m_request.setBody(m_body);
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

bool HttpParser::parseRequestLine() {

  size_t end = m_buffer.find("\r\n", 0);

  // Full request hasn't arrived
  if (end == std::string::npos) {
    return false;
  }

  std::string reqLine = m_buffer.substr(0, end);

  size_t firstSpace = reqLine.find(' ');
  if (firstSpace == std::string::npos) {
    m_state = HttpParserState::ERROR;
    return false;
  }
  size_t secondSpace = reqLine.find(' ', firstSpace + 1);
  if (secondSpace == std::string::npos) {
    m_state = HttpParserState::ERROR;
    return false;
  }
  //TO DO: Method, URI, Version validation
  m_request.setMethod(parseMethod(reqLine.substr(0, firstSpace)));
  m_request.setURI(reqLine.substr(firstSpace + 1, secondSpace - firstSpace - 1));
  m_request.setVersion(reqLine.substr(secondSpace + 1));

  // Remove request line from buffer
  m_buffer.erase(0, end + 2);

  return true;
}


// Assume there is a colon because we check outside
std::string getHeaderName(std::string header) {

  size_t colon = header.find(':');
  std::string name = header.substr(0, colon);
  std::for_each(name.begin(), name.end(), [](char& c) {c = tolower(c);});
  return name;
  
}

std::string getHeaderValue(std::string header) {

  size_t colon = header.find(':');  
  std::string value = header.substr(colon + 1);
  if (!value.empty() && value[0] == ' ')
    value.erase(0, 1);
  return value;
    
}

bool HttpParser::parseHeaders() {

  while (true) {

    size_t end = m_buffer.find("\r\n");

    if (end == std::string::npos)
      return false;

    //Extract one header from the buffer
    std::string header = m_buffer.substr(0, end);
    // Consume the header from the buffer
    m_buffer.erase(0, end + 2);

    if (header.empty()) {
      m_request.setHeaders(m_headers);
      return true;
    }

    size_t colon = header.find(':');
    if (colon == std::string::npos) {
      m_state = HttpParserState::ERROR;
      //TO DO -- set error message
      return false;
    }
    m_headers.insert({getHeaderName(header), getHeaderValue(header)});
  }
}


void HttpParser::determineBodyLength() {

  auto it = m_headers.find("content-length");
  if (it == m_headers.end()) {
    m_expectedBodyLen = 0;
    return;
  }
  m_expectedBodyLen = std::stoi(it->second);
}

bool HttpParser::parseBody() {

  if (m_buffer.size() < m_expectedBodyLen)
  {
      m_body.append(m_buffer);
      m_buffer.clear();
  }

  m_body.append(m_buffer, 0, m_expectedBodyLen);

  m_request.setBody(m_body);

  m_buffer.erase(0, m_expectedBodyLen);

  return true;
}

// ------------ DEBUG -------------------
void HttpParser::printHeaders() {
  for (auto it = m_headers.begin(); it != m_headers.end(); it++) {
    std::cout << it->first << ": " << it->second << std::endl;
  }
}


//------------------------------------ OLD with bool return --------------------

// bool HttpParser::partialParse(const std::string& reqString) {
  
//   size_t start = 0;
  
//   if (!m_reqLine) {
//     size_t firstCRLN = reqString.find("\r\n", 0);
//     if (firstCRLN == std::string::npos) {
//       m_state = HttpParserState::REQUEST_LINE;
//       parseReqLine(m_request, reqString);
//       return true;
//     }
//     parseReqLine(m_request, reqString);
//     m_reqLine = true;
//     start = firstCRLN + 2;
//   }
  
//   size_t headerEnd = reqString.find("\r\n\r\n");
  
//   if (headerEnd == std::string::npos) {
//     m_state = HttpParserState::HEADERS;
//     parsePartHeaders(start, reqString);
//     return true;
//   }
  
//   parsePartHeaders(start, reqString);
  
//   // TODO: First check if we already knew the content length
//   if (m_expectedBodyLen == -1) {
//     int contentLen = -1;
//     auto it = m_headers.find("content-length");
//     if (it == m_headers.end()) {
//       m_body = "";
//       m_state = HttpParserState::COMPLETE;
//       return true;
//     }
//     contentLen = std::stoi(it->second);
//     m_expectedBodyLen = contentLen;
//   }
//   std::string partialBody = reqString.substr(headerEnd + 4);
//   int available = partialBody.length();
//   if (available < m_expectedBodyLen)
//     m_state = HttpParserState::BODY;
//   if (available >= m_expectedBodyLen) // For now not handling a new request starting right after (keep-alive)
//     m_state = HttpParserState::COMPLETE;
//   m_body.append(partialBody);
//   if (m_state == HttpParserState::COMPLETE)
//     buildRequest();

//   return true;

// }

//Assuming there is a full request line passed
// bool HttpParser::parseReqLine(HttpRequest& request, const std::string& reqString) {
  
//   size_t firstSpace = reqString.find(' ', 0);
  
//   //Not even the full method is received
//   if (firstSpace == std::string::npos) {
//     request.setMethod(parseMethod(reqString));
//     return false;
//   }
//   request.setMethod(parseMethod(reqString.substr(0, firstSpace)));
  
//   size_t secondSpace = reqString.find(' ', firstSpace + 1);
//   if (secondSpace == std::string::npos) {
//     request.setURI(reqString.substr(firstSpace + 1));
//     return false;
//   }
//   request.setURI(reqString.substr(firstSpace + 1, secondSpace - firstSpace - 1));
  
//   size_t firstCRLN = reqString.find("\r\n", 0);
//   if (firstCRLN == std::string::npos) {
//     request.setVersion(reqString.substr(secondSpace + 1));
//     return false;
//   }
//   request.setVersion(reqString.substr(secondSpace + 1, firstCRLN - secondSpace - 1));
  
//   return true;
// }


// void HttpParser::parsePartHeaders(int start, const std::string& reqString) {
  
//   size_t end;
  
//   while ((end = reqString.find("\r\n", start)) != std::string::npos) {
//     std::string header = reqString.substr(start, end - start);
//     if (header == "")
//     break;
//     m_headers.insert({getHeaderName(header), getHeaderValue(header)});
//     start = end + 2;
//   }
// }


//------------------------------------ OLD with bool return --------------------


//Should this function always return an HttpRequest, or should it just store
// it and when the parserState == complete, then we can retrieve it?
// HttpRequest HttpParser::parse() {

//   HttpRequest request;

//   parseRequestLine(request);
//   splitHeadersBody();
//   parseHeaders();
//   request.setHeaders(m_headers);
//   request.setBody(m_body);
//   int contentLength = -1;
//   auto it = m_headers.find("Content-Length");
//   if (it != m_headers.end())
//     contentLength = std::stoi(it->second);
//   std::cout << "Content-Length: " << contentLength << std::endl;
//   return request;

// }


// void HttpParser::splitHeadersBody() {
  
//   size_t firstCRLN = this->m_reqText.find("\r\n");
//   size_t headerEnd = this->m_reqText.find("\r\n\r\n");
//   this->m_headersString = this->m_reqText.substr(firstCRLN + 2, headerEnd);
  
  
//   this->m_body = this->m_reqText.substr(headerEnd + 4);
  
  
// }

// bool HttpParser::parseRequestLine(HttpRequest& request){
  
//   size_t firstCRLN = this->m_reqText.find("\r\n", 0);
  
//   std::string firstLine = this->m_reqText.substr(0, firstCRLN);
  
//   size_t firstSpace = firstLine.find(' ', 0);
//   size_t secondSpace = firstLine.find(' ', firstSpace + 1);
  
//   if (firstSpace == std::string::npos || secondSpace == std::string::npos)
//   return false;
  
//   std::string method = firstLine.substr(0, firstSpace);
//   std::string uri = firstLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
//   std::string version = firstLine.substr(secondSpace + 1);
  
//   request.setMethod(parseMethod(method));
//   request.setURI(uri);
//   request.setVersion(version);
  
//   return true;
// }



// void HttpParser::parseHeaders() {
  
//   size_t start = 0;
//   size_t end;
  
//   while ((end = m_headersString.find("\r\n", start)) != std::string::npos) {
//     std::string header = m_headersString.substr(start, end - start);
//     if (header == "")
//     break;
//     m_headers.insert({getHeaderName(header), getHeaderValue(header)});
//     start = end + 2;
//   }
  
// }