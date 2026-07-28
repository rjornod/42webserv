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
		ServerConfig() {setDefaultValues(); std::cout << "server config created\n";}
		~ServerConfig() {};
		void setDefaultValues() {
			m_listenPort = 8080;
			m_serverName = "Test";
			m_clientMaxBodySize = 3500;
	
		}
		int 				getPort() const 															{ return m_listenPort; } 
		std::string getServerName() const 												{ return m_serverName; }
		int 				getClientMaxBody() const 											{ return m_clientMaxBodySize; }
		std::vector<LocationConfig> &getLocationConfigs()					{ return locationConfigs;}
		void				setPort(int port)															{ m_listenPort = port; }
		void				setServerName(std::string name)								{ m_serverName = name; }
		void				setIndex(std::string index)										{ m_index.emplace_back(index);}
		void				setRoot(std::string root)											{ m_root = root;}
		void				setBodySize(int size)													{ m_clientMaxBodySize = size;}
		void				setAutoIndex(bool isOn)												{ m_autoIndex = isOn;}
		void				setErrorPages(int error)											{ m_errorPages.try_emplace(error);}
		// void				setErrorPagesPath(std::string errorPagePath)	{ m_errorPages.emplace(errorPagePath);}
		void				createLocationConfig()												{ locationConfigs.emplace_back();}
		void				printIndex() const {
			std::cout << GREEN << "INDEX: " << RESET;
			for (unsigned long i = 0; i < m_index.size(); i++) {
				std::cout << m_index[i] << " ";
			}
			std::cout << "\n";
		}
		void				printErrorPages() const {
			// std::cout << GREEN << "ERROR PAGES: " << RESET;
			// for (int i = 0; i < m_errorPages.size(); i++) {
			// 	std::cout << m_errorPages[i] << " ";
			// }
			// std::cout << "\n";
		}
		void 				printValues() const {
			std::cout << GREEN << "PORT: " << RESET << m_listenPort 
								<< GREEN << "\nSERVER NAME: " << RESET << m_serverName
								<< GREEN << "\nROOT: " << RESET << m_root
								// << GREEN << "\nINDEX: " << RESET << printIndex()
								<< GREEN << "\nAUTOINDEX: " << RESET << m_autoIndex  
								<< GREEN << "\nCLIENT MAX BODY SIZE: " << RESET << m_clientMaxBodySize << std::endl;
					printIndex();
					printErrorPages();
			}
		};
