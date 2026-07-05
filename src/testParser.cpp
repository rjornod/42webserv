#include "../include/HttpParser.hpp"

int main() {

  std::string request =
    "GET https://medium.com/ HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "User-Agent: test\r\n"
    "Content-Length: 5\r\n"
    "\r\n"
    "hello";

  HttpParser parser;

  parser.partialParse(request);

  std::string state = to_string(parser.getParserState());
  // parser.printHeaders();
  std::cout << "State: " << state << std::endl;

  std::cout << "------------------------- Request: ------------------" << std::endl << parser.getRequest() << std::endl;
  std::cout << "-----------------------------------------------------" << std::endl;
  return 0;

}