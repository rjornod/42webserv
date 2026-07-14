#pragma once
#include <iostream>
#include <vector>
#include "LocationConfig.hpp"

class ServerConfig {
	private:
		std::vector<LocationConfig>	locationConfigs;
		int 												m_port;
		std::string									m_host;
		std::string									m_serverName;
		int 												m_clientMaxBodySize;
	public:
		ServerConfig() {setDefaultValues();}
		~ServerConfig() {};
		void setDefaultValues() {
			m_port = 8080;
			m_host = "127.0.0.1";
			m_serverName = "Test";
			m_clientMaxBodySize = 3500;
			locationConfigs.emplace_back(); // calls default constructor of LocationConfig
		}
		int 				getPort() const 					{ return m_port; } 
		std::string getHost() const 					{ return m_host; }
		std::string getServerName() const 		{ return m_serverName; }
		int 				getClientMaxBody() const 	{ return m_clientMaxBodySize; }
		void 				printValues() const {
			std::cout << "PORT: " << m_port << "\nHOST: " << m_host << "\nSERVER NAME: " << m_serverName << "\nCLIENT MAX BODY SIZE: " << m_clientMaxBodySize << std::endl;
		}
};
