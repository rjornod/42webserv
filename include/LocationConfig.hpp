#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "Colors.hpp"

class LocationConfig {
	private:
		std::string 													m_path;
		std::string 													m_root;
		bool																	m_autoIndex;
		std::vector<std::string>							m_allowedMethods;
		int																		m_maxBodySize;
		std::vector<std::string>							m_index;
		std::map<int, std::string>						m_errorPages;
		std::string														m_upload_store;
		std::pair<int, std::string>						m_return;
		


	public:
		LocationConfig() 														{std::cout << "LOCATION object created\n"; setDefaultValues(); };
		~LocationConfig() 													{};
		LocationConfig(const LocationConfig& other)	: m_path(other.m_path),
      m_root(other.m_root),
      m_autoIndex(other.m_autoIndex),
      m_allowedMethods(other.m_allowedMethods),
      m_maxBodySize(other.m_maxBodySize),
      m_index(other.m_index),
      m_errorPages(other.m_errorPages),
      m_upload_store(other.m_upload_store),
      m_return(other.m_return) { 
				// std::cout << "LocationConfig copy constructor called" << std::endl;
		}
		std::map<int, std::string>	&getErrorPages()				{ return m_errorPages;}
		void	setRoot(std::string root)											{ m_root = root;}
		void	setAutoIndex(bool isOn)												{ m_autoIndex = isOn;}
		void	setBodySize(int size)													{ m_maxBodySize = size;}
		void	setIndex(std::string index)										{ m_index.emplace_back(index);}
		void	setErrorPages(int error, std::string path)		{ m_errorPages.emplace(error, path);}
		void	setUploadStore(std::string path)							{ m_upload_store = path;}
		void	setAllowedMethod(std::string method)					{ m_allowedMethods.emplace_back(method);}
		void	setReturn(int code, std::string path)					{ m_return.first = code; m_return.second = path;} // TO DO: maybe change the variable name
		void 	clearDefaultMethods()													{ m_allowedMethods.clear();}
		void 	setDefaultValues() {
			m_path = "/example/path";
			m_root = "/";
			m_autoIndex = false;
			m_allowedMethods.clear();
			m_allowedMethods.push_back("GET");
			m_allowedMethods.push_back("POST");
			m_maxBodySize = 0;
			m_index.clear();
			m_errorPages.clear();
			m_upload_store.clear();
			m_return.first = 0;
			m_return.second.clear();
		}
		void				printValues() const {
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
		void				setPath(std::string path)	{ m_path = path; }
		std::string	getPath()									{ return m_path; }
};