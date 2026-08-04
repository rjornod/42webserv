#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "LocationConfig.hpp"
#include "Colors.hpp"

class ServerConfig {
	private:
		std::vector<LocationConfig>	locationConfigs;
		int 												m_listenPort;
		std::string									m_serverName;
		std::string									m_root;
		int 												m_clientMaxBodySize;
		std::vector<std::string>		m_index;
		std::map<int, std::string>	m_errorPages;
		bool												m_autoIndex;
	public:
		ServerConfig() {setDefaultValues();}
		~ServerConfig() {};
		void setDefaultValues();
		int 				getPort() const 															{ return m_listenPort; } 
		std::string getServerName() const 												{ return m_serverName; }
		int 				getClientMaxBody() const 											{ return m_clientMaxBodySize; }
		std::map<int, std::string>	&getErrorPages()							{ return m_errorPages;}
		std::vector<LocationConfig> &getLocationConfigs()					{ return locationConfigs;}
		void				setPort(int port)															{ m_listenPort = port; }
		void				setServerName(std::string name)								{ m_serverName = name; }
		void				setIndex(std::string index)										{ m_index.emplace_back(index);}
		void				setRoot(std::string root)											{ m_root = root;}
		void				setBodySize(int size)													{ m_clientMaxBodySize = size;}
		void				setAutoIndex(bool isOn)												{ m_autoIndex = isOn;}
		void				setErrorPages(int error, std::string path)		{ m_errorPages.emplace(error, path);}
		void				createLocationConfig()												{ locationConfigs.emplace_back();}
		void				setErrorPath(std::string errorPath)						{ for (auto it = m_errorPages.begin(); it != m_errorPages.end(); ++it)
				it->second = errorPath;
		}
		void 	checkDuplicateLocations(std::string path);
		std::unordered_set<std::string> seenLocations;
		void				printIndex() const;
		void 				printErrorPages() const;
		void 				printValues() const;		
	};
