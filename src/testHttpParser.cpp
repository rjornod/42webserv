#include "HttpParser.hpp"
#include "Router.hpp"
#include "GlobalConfig.hpp"
#include "ConfigParser.hpp"
#include "RequestProcessor.hpp"
#include "HttpResponse.hpp"
#include "ResponseWriter.hpp"
#include <sys/socket.h>


int main(int argc, char **argv) {

  std::string request =
    "POST /secret_dir/hello.txt HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "User-Agent: test\r\n"
    "Content-Length: 47\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "helloxxxxxxxxxxxxxxxxxxxxxxxxxxxxccccccchfgghfg";

  if (argc != 2) {
    std::cout << "Usage ./webserv <path/to/configfile>" << std::endl;
    exit(-1);
  }

  HttpParser parser;

  parser.parse(request);

  std::cout << "Parser state : " << to_string(parser.getParserState()) << std::endl;

  std::cout << "------------------------- Request: ------------------" << std::endl << parser.getRequest() << std::endl;
  std::cout << "-----------------------------------------------------" << std::endl;

  (void)argc;
  if (parser.getParserState() != HttpParserState::COMPLETE &&
      parser.getParserState() != HttpParserState::ERROR) {
    std::cout << "Request incomplete" << std::endl;
    exit(0);
  }
  

  GlobalConfig globalConfig;
  ConfigParser config(argv[1], globalConfig);
	if (!config.processConfig()) {
    std::cout << "Error in parsing the config file" << std::endl;
	}
  
  // for (unsigned long i = 0; i < globalConfig.getServerConfigs().size(); i++) {
	// 	std::cout << GREEN << "*************** Index: " << i << " ***************" << RESET << std::endl;
	// 	globalConfig.getServerConfigs()[i].printValues(); 
	// 	// std::cout << GREEN << "****************************************" << RESET << std::endl;
	// }

  Router router;
  RequestContext ctx = router.createContext(parser.getRequest(), globalConfig, 0);

  if (ctx.getLocationConfig()) {
    std::cout << "Request context is: " << std::endl;
    ctx.getLocationConfig()->printValues();
  }
  else {
    std::cout << "Request context is global: " << std::endl;
    ctx.getServerConfig()->printValues();
  }

  RequestProcessor processor;
  HttpResponse response = processor.process(ctx);

  ResponseWriter writer = ResponseWriter(response);
  int sv[2];
  socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
  // sv[0] and sv[1] are now connected to each other, like a socket connection

  // Pretend sv[0] is "the client's socket" that your server writes to
  while (!writer.isDone()) {
      writer.writeTo(sv[0]); 
  }

  // sv[1] is "the other end" — read from it to verify what got sent
  char buf[4096];
  ssize_t n = read(sv[1], buf, sizeof(buf));
  printf("Received %zd bytes:\n%.*s\n", n, (int)n, buf);

  close(sv[0]);
  close(sv[1]);

  return 0;

}