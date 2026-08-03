#include "../include/HttpParser.hpp"
#include "../include/HttpMethod.hpp"

void HttpParser::reportErrors() {
  if (m_state == HttpParserState::ERROR)
    std::cerr << "Error on parsing: " << m_errorMessage << std::endl;
}

void HttpParser::parse(std::string_view chunk) {

  m_buffer += chunk;

  switch (getParserState()) {
    case HttpParserState::REQUEST_LINE:
      if (!parseRequestLine()) {
        reportErrors();
        return;
      }
      m_state = HttpParserState::HEADERS;
      [[fallthrough]];

    case HttpParserState::HEADERS:
      if (!parseHeaders()) {
        reportErrors();
        return;
      }
      determineBodyLength();
      if (m_expectedBodyLen == 0) {
        m_request.setBody("");
        m_state = HttpParserState::COMPLETE;
        return;
      }
      if (m_expectedBodyLen == -1) {
        reportErrors();
        return;
      }
      m_state = HttpParserState::BODY;
      [[fallthrough]];
    
    case HttpParserState::BODY:
      if (!parseBody()) {
        reportErrors();
        return;
      }
      m_state = HttpParserState::COMPLETE;
      [[fallthrough]];

    case HttpParserState::COMPLETE:
      break;

    //Not sure if it will ever fall here (reportErrors is triggered on errors)
    case HttpParserState::ERROR:
      std::cout << "Error on parsing: " << m_errorMessage << std::endl;
      
    default:
      break;
  }
}

HttpMethod parseMethod(std::string_view method) {
  
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

  std::string_view reqLine = std::string_view(m_buffer).substr(0, end);

  size_t firstSpace = reqLine.find(' ');
  if (firstSpace == std::string_view::npos) {
    m_state = HttpParserState::ERROR;
    m_errorMessage = "Missing URI";
    return false;
  }
  size_t secondSpace = reqLine.find(' ', firstSpace + 1);
  if (secondSpace == std::string_view::npos) {
    m_state = HttpParserState::ERROR;
    m_errorMessage = "Missing HTTP version";
    return false;
  }
  //TO DO: Method, URI, Version validation
  // Method
  m_request.setMethod(parseMethod(reqLine.substr(0, firstSpace)));
  if (m_request.getMethod() == HttpMethod::UNKNOWN) {
    m_state = HttpParserState::ERROR;
    m_errorMessage = "Unsupported method";
    return false;
  }
  m_request.setURI(reqLine.substr(firstSpace + 1, secondSpace - firstSpace - 1));
  std::string_view version = reqLine.substr(secondSpace + 1);
  if (!validateHttpVersion(version)) {
    m_state = HttpParserState::ERROR;
    m_errorMessage = "Invalid Http Version";
    return false;
  } 
  m_request.setVersion(version);

  // Remove request line from buffer
  m_buffer.erase(0, end + 2);

  return true;
}

// Can assume there is a colon because we check outside
std::string getHeaderName(std::string_view header) {

  size_t colon = header.find(':');
  std::string name = static_cast<std::string>(header.substr(0, colon));
  std::for_each(name.begin(), name.end(), [](char& c) {c = tolower(c);});
  return std::string(name);
  
}

std::string getHeaderValue(std::string_view header) {

  size_t colon = header.find(':');

  std::string_view value = header.substr(colon + 1);
  if (!value.empty() && 
      (value.front() == ' ' || value.front() == '\t'))
    value.remove_prefix(1);
  return std::string(value);
    
}

bool HttpParser::parseHeaders() {

  while (true) {
    size_t end = m_buffer.find("\r\n");

    if (end == std::string::npos)
      return false;

    //Extract one header from the buffer
    std::string_view header = std::string_view(m_buffer).substr(0, end);

    if (header.empty()) {
      m_buffer.erase(0, 2);
      m_request.setHeaders(m_headers);
      return true;
    }

    size_t colon = header.find(':');
    if (colon == std::string_view::npos) {
      m_state = HttpParserState::ERROR;
      m_errorMessage = "Header missing colon";
      return false;
    }
    m_headers.insert({getHeaderName(header), getHeaderValue(header)});

    // Consume the header from the buffer
    m_buffer.erase(0, end + 2);
  }
}


void HttpParser::determineBodyLength() {

  auto it = m_headers.find("content-length");
  if (it == m_headers.end()) {
    m_expectedBodyLen = 0;
    return;
  }
  try {
    m_expectedBodyLen = std::stoi(it->second);
    if (m_expectedBodyLen < 0) {
      m_errorMessage = "Content-Length Out Of Range";
      m_state = HttpParserState::ERROR;
      m_expectedBodyLen = -1;
      return;
    }
  }
  catch (const std::invalid_argument& ia) {
    m_errorMessage = "Invalid Content-Length";
    m_state = HttpParserState::ERROR;
    m_expectedBodyLen = -1;
    return;
  }
  catch (const std::out_of_range& oor) {
    m_errorMessage = "Content-Length Out Of Range";
    m_state = HttpParserState::ERROR;
    m_expectedBodyLen = -1;
    return;
  }
  catch (const std::exception& e) {
    m_errorMessage = "Unknown error on Content-Length argument";
    m_state = HttpParserState::ERROR;
    m_expectedBodyLen = -1;
    return;
  }
  
}

bool HttpParser::parseBody() {

  if (m_expectedBodyLen == -1) {
    return false;
  }
  if (m_buffer.size() + m_body.size() < m_expectedBodyLen) {
      m_body.append(m_buffer);
      m_buffer.clear();
      return false;
  }
  m_body.append(m_buffer, 0, m_expectedBodyLen);
  m_request.setBody(m_body);
  m_buffer.erase(0, m_expectedBodyLen);

  return true;
}

// VALIDATION ------

bool isDigits(std::string_view s) {
  return !s.empty() &&
           std::all_of(s.begin(), s.end(),
                       [](unsigned char c) { return std::isdigit(c); });
}

bool HttpParser::validateHttpVersion(std::string_view version) {

  if (version.length() < 8)
    return false;
  std::string_view http = version.substr(0,5);
  if (http != "HTTP/")
    return false;
  size_t point = version.find('.');
  if (point == std::string_view::npos)
    return false;
  std::string_view major = version.substr(5, point - 5);
  std::string_view minor = version.substr(point + 1);
  if (!isDigits(major) || !isDigits(minor))
    return false;
  return true;
}

void HttpParser::clearParser() {
  *this = HttpParser{};
}

// ------------ DEBUG -------------------
void HttpParser::printHeaders() {
  for (auto it = m_headers.begin(); it != m_headers.end(); it++) {
    std::cout << it->first << ": " << it->second << std::endl;
  }
}