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
    "helloPOST /nextreq";

  std::string reqWithBody =
    "POST / HTTP/1.1\r\n"
    "Host: developer.mozilla.org\r\n"
    "User-Agent: curl/8.6.0\r\n"
    "Accept: */*\r\n"
    "Content-Type: application/json\r\n"
    "content-length: 12\r\n"
    "\r\n"
    "{\"id\": \"42\"}";

  std::string missingVersion =
    "GET /index.html\r\n"
    "Host: localhost\r\n"
    "User-Agent: Test\r\n"
    "\r\n";

  // HttpParser parser;

  // parser.partialParse(reqMethod);
  // parser.partialParse(reqLineURI);
  // parser.partialParse(reqHeaders);
  // std::string state = to_string(parser.getParserState());
  // std::cout << "State: " << state << std::endl;

  // std::cout << "------------------------- Request: ------------------" << std::endl << parser.getRequest() << std::endl;
  // std::cout << "-----------------------------------------------------" << std::endl;

  // std::cout << "Buffer:" << std::endl << parser.getBuffer() << std::endl;

  std::cout << std::endl << "Parsing request with missing version: " << std::endl << std::endl;

  HttpParser parserError;

  parserError.partialParse(missingVersion);


  std::cout << "------------------------- Request: ------------------" << std::endl << parserError.getRequest() << std::endl;
  std::cout << "-----------------------------------------------------" << std::endl;
  std::cout << "State: " << to_string(parserError.getParserState()) << std::endl;

  return 0;

}