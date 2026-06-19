#include "../include/HttpRequest.hpp"


std::ostream &operator<<(std::ostream &out, const HttpRequest &request) {

  out << "Method: " << request.getMethod() << std::endl;
  out << "URI: " << request.getURI() << std::endl;
  out << "Version: " << request.getVersion() << std::endl;
  out << "Body: " << request.getBody();

  return out;
}