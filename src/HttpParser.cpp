#include "../include/HttpParser.hpp"
#include "../include/HttpMethod.hpp"

void HttpParser::reportErrors() {
  if (m_state == HttpParserState::ERROR)
    std::cout << "Error on parsing: " << m_errorMessage << std::endl;
}

void HttpParser::partialParse(const std::string& chunk) {

  m_buffer += chunk;

  switch (getParserState()) {
    case HttpParserState::REQUEST_LINE:
      if (!parseRequestLine()) {
        reportErrors();
        return;
      }

      m_state = HttpParserState::HEADERS;

    case HttpParserState::HEADERS:
      if (!parseHeaders()) {
        reportErrors();
        return;
      }
      determineBodyLength();
      if (m_expectedBodyLen == 0) {
        m_request.setBody("");
        m_state = HttpParserState::COMPLETE;
        // buildRequest(); // Not sure
        return;
      }
      m_state = HttpParserState::BODY;
    
    case HttpParserState::BODY:
      if (!parseBody()) {
        reportErrors();
        return;
      }
      m_state = HttpParserState::COMPLETE;

    case HttpParserState::COMPLETE:
      break;

    case HttpParserState::ERROR:
      std::cout << "Error on parsing: " << m_errorMessage << std::endl;
      
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
    m_errorMessage = "Missing URI";
    return false;
  }
  size_t secondSpace = reqLine.find(' ', firstSpace + 1);
  if (secondSpace == std::string::npos) {
    m_state = HttpParserState::ERROR;
    m_errorMessage = "Missing HTTP version";
    return false;
  }
  //TO DO: Method, URI, Version validation
  m_request.setMethod(parseMethod(reqLine.substr(0, firstSpace)));
  if (m_request.getMethod() == HttpMethod::UNKNOWN) {
    m_state = HttpParserState::ERROR;
    m_errorMessage = "Invalid method";
    return false;
  }
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

  if (m_buffer.size() + m_body.size() < m_expectedBodyLen)
  {
      m_body.append(m_buffer);
      m_buffer.clear();
      return false;
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