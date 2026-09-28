#include "HttpResponse.hpp"

// HttpResponse &HttpResponse::operator=(const HttpResponse & other){

// }
void HttpResponse::makeStatusLine(){

  std::string statusLine = "HTTP/1.1 " + std::to_string(m_statusCode) + " " + getReasonPhrase() + "\r\n";
  setStatusLine(statusLine);
}


void HttpResponse::addHeader(std::string name, std::string value) {
  m_headers.push_back({name , value});
}


std::ostream &operator<<(std::ostream &out, const HttpResponse &response) {

  out << "[Status line] " << std::endl << response.getStatusLine() << std::endl;
  out<< "[Headers] " << std::endl;

  std::vector<std::pair<std::string, std::string>> headers = response.getHeaders();
  for (auto it = headers.begin(); it != headers.end(); it++) {
    out << it->first << ": " << it->second << std::endl;
  }

  std::string body;
  if (std::holds_alternative<std::string>(response.getbodySource()))
    body = "[string] " + std::get<std::string>(response.getbodySource());
  else
    body =  "[path] " + std::get<std::filesystem::path>(response.getbodySource()).string();
  

  out  << "[Body]" << std::endl << body << std::endl;
  return out;
}

const std::map<int, std::string> HttpResponse::M_STATUSCODES = {
  {200, "OK"},
  {400, "Bad Request"},
  {403, "Forbidden"},
  {404, "Not found"},
  {405, "Method Not Allowed"},
  {408, "Request Timeout"},
  {413, "Content Too Large"},
  {418, "I'm a teapot"}, 
  {500, "Internal Server Error"},
  {505, "HTTP Version Not Supported"}
};