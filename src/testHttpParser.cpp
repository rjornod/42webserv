#include "HttpParser.hpp"
#include "Router.hpp"
#include "GlobalConfig.hpp"
#include "ConfigParser.hpp"
#include "RequestProcessor.hpp"
#include "HttpResponse.hpp"

int main(int argc, char **argv) {

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

  std::string invalidContentLen = 
    "POST / HTTP/1.1\r\n"
    "Host: developer.mozilla.org\r\n"
    "User-Agent: curl/8.6.0\r\n"
    "Accept: */*\r\n"
    "Content-Type: application/json\r\n"
    "content-length: 0\r\n"
    "\r\n"
    "{\"id\": \"42\"}";

  std::string reqRouting =
    "POST /images/logo.png HTTP/1.1\r\n"
    "Host: developer.mozilla.org\r\n"
    "User-Agent: curl/8.6.0\r\n"
    "Accept: */*\r\n"
    "Content-Type: application/json\r\n"
    "content-length: 12\r\n"
    "\r\n"
    "{\"id\": \"42\"}";

  if (argc != 2) {
    std::cout << "Usage ./webserv <path/to/configfile>" << std::endl;
    exit(-1);
  }

  HttpParser parser;

  parser.parse(reqRouting);
  // std::string state = to_string(parser.getParserState());
  // std::cout << "State: " << state << std::endl;

  std::cout << "------------------------- Request: ------------------" << std::endl << parser.getRequest() << std::endl;
  std::cout << "-----------------------------------------------------" << std::endl;

  std::cout << "Buffer:" << std::endl << parser.getBuffer() << std::endl;
  GlobalConfig globalConfig;
  ConfigParser config(argv[1], globalConfig);
	if (!config.processConfig()) {
    std::cout << "Error in parsing the config file" << std::endl;
	}
  (void)argc;
  
  for (unsigned long i = 0; i < globalConfig.getServerConfigs().size(); i++) {
		std::cout << GREEN << "*************** Index: " << i << " ***************" << RESET << std::endl;
		globalConfig.getServerConfigs()[i].printValues(); 
		// std::cout << GREEN << "****************************************" << RESET << std::endl;
	}

  Router router;
  RequestContext ctx = router.createContext(parser.getRequest(), globalConfig, 0);

  // std::cout << "Location path: " << ctx.getLocationConfig()->getPath() << std::endl;

  std::cout << "Request context is: " << std::endl;
  ctx.getLocationConfig()->printValues();

  RequestProcessor processor;
  HttpResponse response = processor.process(ctx);

  std::cout << "Response status code: "<< response.getStatusCode() << std::endl;

  return 0;

}