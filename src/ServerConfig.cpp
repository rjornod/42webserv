#include "ServerConfig.hpp"
#include "ConfigParseException.hpp"
#include "ServerConfig.hpp"

void ServerConfig::checkDuplicateLocations(const std::string& path) {
	if (!m_seenLocations.emplace(path).second)
		throw ConfigParseException("Duplicate locations not allowed: '" + path + "'");
}

void ServerConfig::setDefaultValues() {
	m_root = "/www";
	m_clientMaxBodySize = 100000000;
	m_autoIndex = false;
	m_errorPages.clear();
}

void ServerConfig::seenDirective(std::string directive) {
	m_seendirectives.emplace(directive);
}
/**
 * Function checks if each server block has all the necessary directives;
 * Currently only listen is mandatory but more can be added here later;
 */
void ServerConfig::checkMandatoryDirectives() {
	if (m_seendirectives.find("listen") == m_seendirectives.end())
		throw ConfigParseException("Detected server without a 'listen' directive.\nMake sure every server has 'listen' directive!");
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
	// for (auto it = m_cgiHandlers.begin(); it != m_cgiHandlers.end(); it++) {
	// 	std::cout << it->first << ", " << it->second << "\n";
	// }
	// printIndex();
	// printErrorPages();
	std::cout << YELLOW << "LOCATION CONFIGS: " << RESET;
	if (m_locationConfigs.empty()) {
		std::cout << "(not set)" << std::endl;
	} else {
		std::cout << std::endl;
		for (const LocationConfig& location : m_locationConfigs)
			location.printValues();
	}
}
