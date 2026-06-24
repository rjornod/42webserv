#include "Client.hpp"
#include "Server.hpp"
#include "ConfigParser.hpp"
#include "GlobalConfig.hpp"

void exitProgram(int errorCode, std::string reason) {
	std::cerr << "Error: " << reason << std::endl;
	exit(errorCode);
}


int main(int argc, char **argv) {
	if (argc < 2)
		exitProgram(-1, "Usage ./webserv <path/to/configfile>");
	ConfigParser config(argv[1]); 
	config.parseFile();
	/**
	 * FIXED: Copies are being made when adding objects to a vector
	 */
	GlobalConfig globalConfig;
	globalConfig.getServerConfigs()[0].printValues(); 
	std::cout << globalConfig.getServerConfigs()[0].getClientMaxBody() << std::endl;
	Server server(globalConfig);
	if (server.serverSetup() > 0) {
		exitProgram(1, "ServerSetup");
	}
	if (server.connections() > 0) {
		exitProgram(1, "Connections");
	}
	server.serverCore();
}
