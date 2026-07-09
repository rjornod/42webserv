#include "../include/HttpRequest.hpp"


std::ostream &operator<<(std::ostream &out, const HttpRequest &request) {

  out << "Method: " << to_string(request.getMethod()) << std::endl;
  out << "URI: " << request.getURI() << std::endl;
  out << "Version: " << request.getVersion() << std::endl;
  out << "Headers: " << std::endl;
  // for (std::string header : request.getHeaders()) 
  //   out << header << std::endl;
  std::unordered_map<std::string, std::string> headers = request.getHeaders();
  for (auto it = headers.begin(); it != headers.end(); it++) {
    out << it->first << ": " << it->second << std::endl;
  }

  out << "Body: " << request.getBody();

  return out;
}