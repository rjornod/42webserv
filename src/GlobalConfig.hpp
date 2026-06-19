#pragma once
#include <iostream>
#include <vector>
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

class GlobalConfig {
	private:
		std::vector<ServerConfig> m_serverConfigs;
	public:
		GlobalConfig() { std::cout << "GlobalConfig object created" <<std::endl; createServerConfig();};
		~GlobalConfig() {std::cout << "GlobalConfig object destroyed" <<std::endl;};
		std::vector<ServerConfig>& getServerConfigs() { return m_serverConfigs; }
		const std::vector<ServerConfig>& getServerConfigs() const { return m_serverConfigs; }
		void createServerConfig() {
			m_serverConfigs.emplace_back();
		}
		int parseConfig(char* pathToFile);
};
