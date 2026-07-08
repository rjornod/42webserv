#include "../include/HttpParser.hpp"

int main() {

  std::string request =
    "GET https://medium.com/ HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "User-Agent: test\r\n"
    "Content-Length: 5\r\n"
    "\r\n"
    "hello";


  std::string reqMethod = 
    "GET ";

  std::string reqLineURI = 
    "https://medium.com/ HTTP/1.1\r\n"
    "Host: example.com\r\n";

  std::string reqHeaders = 
    "User-Agent: test\r\n"
    "Content-Length: 5\r\n"
    "\r\n"
    "helloGET /nextreq";

  HttpParser parser;

  parser.partialParse(reqMethod);
  parser.partialParse(reqLineURI);
  parser.partialParse(reqHeaders);
  std::string state = to_string(parser.getParserState());
  std::cout << "State: " << state << std::endl;

  std::cout << "------------------------- Request: ------------------" << std::endl << parser.getRequest() << std::endl;
  std::cout << "-----------------------------------------------------" << std::endl;

  std::cout << "Buffer:" << std::endl << parser.getBuffer() << std::endl;

  return 0;

}