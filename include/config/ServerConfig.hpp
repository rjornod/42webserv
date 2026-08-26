#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include "LocationConfig.hpp"
#include "Colors.hpp"

class ServerConfig {
	private:
		std::vector<LocationConfig>										m_locationConfigs;
		int 																					m_listenPort;
		std::string																		m_serverName;
		std::string																		m_root;
		int 																					m_clientMaxBodySize;
		std::vector<std::string>											m_index;
		std::map<int, std::string>										m_errorPages;
		bool																					m_autoIndex;
		std::unordered_map<std::string, std::string>	m_cgiHandlers;
		std::unordered_set<std::string> 							m_seenLocations;
		std::unordered_set<std::string> 							m_seendirectives;
	public:
		ServerConfig() 																								{ setDefaultValues(); }
		~ServerConfig() 																							{};
		int 				getPort() const 																	{ return m_listenPort; } 
		std::string getServerName() const 														{ return m_serverName; }
		int 				getClientMaxBody() const 													{ return m_clientMaxBodySize; }
		std::map<int, std::string>	&getErrorPages()									{ return m_errorPages;}
		std::vector<LocationConfig> &getLocationConfigs()							{ return m_locationConfigs;}
		const std::vector<LocationConfig> &getLocationConfigs() const { return m_locationConfigs;}
		void				setPort(int port)																	{ m_listenPort = port; }
		void				setServerName(std::string& name)									{ m_serverName = name; }
		void				setIndex(std::string& index)											{ m_index.emplace_back(index);}
		void				setRoot(std::string& root)												{ m_root = root;}
		void				setBodySize(int size)															{ m_clientMaxBodySize = size;}
		void				setErrorPages(int error, std::string& path)				{ m_errorPages.emplace(error, path);}
		void				setAutoIndex(bool isOn)														{ m_autoIndex = isOn;}
		void				setCgiHandler(std::string ext, std::string path)	{ m_cgiHandlers.try_emplace(ext, path); }
		void				createLocationConfig()														{ m_locationConfigs.emplace_back();}
		void 				setDefaultValues();
		void 				checkDuplicateLocations(const std::string& path);
		void				seenDirective(std::string directive);
		void				checkMandatoryDirectives();
		void				printIndex() const;
		void 				printErrorPages() const;
		void 				printValues() const;		
	};
