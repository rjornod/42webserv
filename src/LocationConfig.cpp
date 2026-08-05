#include "../include/LocationConfig.hpp"
#include "../include/ConfigParseException.hpp"

void LocationConfig::setDefaultValues() {
	m_path = "/example/path";
	m_root = "/";
	m_autoIndex = false;
	m_allowedMethods.clear();
	m_allowedMethods.push_back("GET");
	m_maxBodySize = 100000000;
	m_index.clear();
	m_errorPages.clear();
	m_upload_store = "www/uploads";
	m_return.first = 0;
	m_return.second.clear();
}

// DEBUG FUNCTIONS
void LocationConfig::printValues() const {
	std::cout << YELLOW << "\n__________ LOCATION DETAILS ___________\n"
						<< YELLOW << "PATH: " << RESET << m_path
						<< YELLOW << "\nROOT: " << RESET << (m_root.empty() ? "(not set)" : m_root)
						<< YELLOW << "\nAUTOINDEX: " << RESET << m_autoIndex
						<< YELLOW << "\nMAX BODY SIZE: " << RESET << m_maxBodySize
						<< YELLOW << "\nUPLOAD STORE: " << RESET << (m_upload_store.empty() ? "(not set)" : m_upload_store)
						<< YELLOW << "\nRETURN: " << RESET;
	if (m_return.first != 0)
		std::cout << m_return.first << " -> " << m_return.second;
	else
		std::cout << "(not set)";
	std::cout << std::endl;
	std::cout << YELLOW << "ALLOWED METHODS: " << RESET;
	if (m_allowedMethods.empty())
		std::cout << "(not set)";
	else
		for (const std::string& method : m_allowedMethods)
			std::cout << method << " ";
	std::cout << std::endl;
	std::cout << YELLOW << "INDEX: " << RESET;
	if (m_index.empty())
		std::cout << "(not set)";
	else
		for (const std::string& index : m_index)
			std::cout << index << " ";
	std::cout << std::endl;
	std::cout << YELLOW << "ERROR PAGES: " << RESET;
	if (m_errorPages.empty())
		std::cout << "(not set)";
	else
		for (const auto& pair : m_errorPages)
			std::cout << pair.first << " : " << pair.second << " ";
	std::cout << std::endl;
}