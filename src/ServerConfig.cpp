#include "../include/ServerConfig.hpp"
#include "../include/ConfigParseExecption.hpp"

void ServerConfig::checkDuplicateLocations(std::string path) {
	if (!seenLocations.emplace(path).second)
		throw ConfigParseException("Duplicate locations not allowed: '" + path + "'");
}

void ServerConfig::setDefaultValues() {
	m_listenPort = 8080;
	m_serverName = "Test";
	m_root.clear();
	m_clientMaxBodySize = 3500;
	m_autoIndex = false;
	m_index.clear();
	m_errorPages.clear();
}


//	DEBUG FUNCTIONS
void ServerConfig::printIndex() const {
	std::cout << YELLOW << "INDEX: " << RESET;
	if (m_index.empty()) {
		std::cout << "(not set)";
	} else {
		for (unsigned long i = 0; i < m_index.size(); i++) {
			std::cout << m_index[i] << " ";
		}
	}
	std::cout << "\n";
}

void ServerConfig::printErrorPages() const {
	std::cout << YELLOW << "ERROR PAGES: " << RESET;
	if (m_errorPages.empty()) {
		std::cout << "(not set)" << std::endl;
		return;
	}
	for (const auto& pair : m_errorPages)
	{
			std::cout << pair.first << " : " << pair.second << std::endl;
	}
}

void ServerConfig::printValues() const {
	std::cout << GREEN << "\n__________ SERVER DETAILS ___________\n"
						<< YELLOW << "PORT: " << RESET << m_listenPort
						<< YELLOW << "\nSERVER NAME: " << RESET << (m_serverName.empty() ? "(not set)" : m_serverName)
						<< YELLOW << "\nROOT: " << RESET << (m_root.empty() ? "(not set)" : m_root)
						<< YELLOW << "\nAUTOINDEX: " << RESET << m_autoIndex
						<< YELLOW << "\nCLIENT MAX BODY SIZE: " << RESET << m_clientMaxBodySize << std::endl;
			// printIndex();
			// printErrorPages();
			// std::cout << YELLOW << "LOCATION CONFIGS: " << RESET;
			// if (locationConfigs.empty()) {
			// 	std::cout << "(not set)" << std::endl;
			// } else {
			// 	std::cout << std::endl;
			// 	for (const LocationConfig& location : locationConfigs)
			// 		location.printValues();
			// }
}
