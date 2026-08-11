#include "../include/server/Client.hpp"
#include "../include/server/Server.hpp"
#include "../include/config/ConfigParser.hpp"
#include "../include/config/GlobalConfig.hpp"

void exitProgram(int errorCode, std::string reason) {
	std::cerr << "Error: " << reason << std::endl;
	exit(errorCode);
}

int main(int argc, char **argv) {
	if (argc != 2)
		exitProgram(-1, "Usage ./webserv <path/to/configfile>");
	GlobalConfig globalConfig;
	ConfigParser config(argv[1], globalConfig);
	if (config.processConfig()) {
		exitProgram(1, "ConfigParser");
	}

	/* For Debug: Prints the value of all the configurations of server and location*/
	for (unsigned long i = 0; i < globalConfig.getServerConfigs().size(); i++) {
		globalConfig.getServerConfigs()[i].printValues(); 
	}
	Server server(globalConfig);
	if (server.serverSetup() > 0) {
		exitProgram(1, "ServerSetup");
	}
	if (server.connections() > 0) {
		exitProgram(1, "Connections");
	}
	server.serverCore();
}
