#pragma once
#include <iostream>
#include <vector>
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

class GlobalConfig {
	private:
		std::vector<ServerConfig> m_serverConfigs;
	public:
		GlobalConfig() 																						{};
		~GlobalConfig() {};
		std::vector<ServerConfig>& getServerConfigs() 						{ return m_serverConfigs; }
		const std::vector<ServerConfig>& getServerConfigs() const { return m_serverConfigs; }
		ServerConfig& createServerConfig() 												{ m_serverConfigs.emplace_back(); return m_serverConfigs.back(); }
};
