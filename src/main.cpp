#include "../include/Client.hpp"
#include "../include/Server.hpp"
#include "../include/ConfigParser.hpp"
#include "../include/GlobalConfig.hpp"

void exitProgram(int errorCode, std::string reason) {
	std::cerr << "Error: " << reason << std::endl;
	exit(errorCode);
}


int main(int argc, char **argv) {
	if (argc < 2)
		exitProgram(-1, "Usage ./webserv <path/to/configfile>");
	GlobalConfig globalConfig;
	ConfigParser config(argv[1], globalConfig); 
	if (config.processConfig()) {
		exitProgram(1, "ConfigParser");
	}
	globalConfig.getServerConfigs()[0].printValues(); 
	Server server(globalConfig);
	if (server.serverSetup() > 0) {
		exitProgram(1, "ServerSetup");
	}
	if (server.connections() > 0) {
		exitProgram(1, "Connections");
	}
	server.serverCore();
}
