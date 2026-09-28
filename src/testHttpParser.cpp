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
    "GET secret.html HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "User-Agent: test\r\n"
    "Content-Length: 5\r\n"
    "\r\n"
    "hello";

  if (argc != 2) {
    std::cout << "Usage ./webserv <path/to/configfile>" << std::endl;
    exit(-1);
  }

  HttpParser parser;

  parser.parse(request);

  // std::cout << "------------------------- Request: ------------------" << std::endl << parser.getRequest() << std::endl;
  // std::cout << "-----------------------------------------------------" << std::endl;

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

  std::cout << "Request context is: " << std::endl;
  if (ctx.getLocationConfig())
    ctx.getLocationConfig()->printValues();
  else
    std::cout << "No suitable location found" << std::endl;
  

  RequestProcessor processor;
  HttpResponse response = processor.process(ctx);

  std::cout << std::endl << "Response: " << std::endl << response << std::endl;

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