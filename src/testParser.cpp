#include "../include/HttpParser.hpp"

int main() {

  std::string request =
    "GET https://medium.com/ HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "User-Agent: test\r\n"
    "\r\n";

  HttpParser parser(request);

  // parser.splitHeadersBody();

  // std::vector<std::string> headers = parser.split_lines(parser.getHeaders());

  // std::cout << "Headers: " << std::endl;
  // for (std::string line : headers)
  //   std::cout << line << std::endl;

  // std::cout << "Body: " << parser.getBody() << std::endl;

  HttpRequest req = parser.parse();

  std::cout << "------------------------- Request: ------------------" << std::endl << req << std::endl;

  return 0;

}