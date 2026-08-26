#pragma once
#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include "Colors.hpp"

class LocationConfig {
	private:
		std::string 																	m_path;
		std::string 																	m_root;
		bool																					m_autoIndex;
		std::vector<std::string>											m_allowedMethods;
		int																						m_maxBodySize;
		std::vector<std::string>											m_index;
		std::map<int, std::string>										m_errorPages;
		std::string																		m_upload_store;
		std::pair<int, std::string>										m_return;
		std::unordered_map<std::string, std::string>	m_cgiHandlers;
	public:
		LocationConfig() 																		{ setDefaultValues(); };
		~LocationConfig() 																			{};
		std::map<int, std::string>	&getErrorPages()						{ return m_errorPages;}
		std::string getRoot()															const	{ return m_root;}
		void	setRoot(std::string root)													{ m_root = root;}
		void	setAutoIndex(bool isOn)														{ m_autoIndex = isOn;}
		void	setBodySize(int size)															{ m_maxBodySize = size;}
		void	setIndex(std::string& index)											{ m_index.emplace_back(index);}
		void	setErrorPages(int error, std::string& path)				{ m_errorPages.emplace(error, path);}
		void	setUploadStore(std::string& path)									{ m_upload_store = path;}
		void	setAllowedMethod(std::string& method)							{ m_allowedMethods.emplace_back(method);}
		void 	setCgiHandler(std::string ext, std::string path)	{ m_cgiHandlers.try_emplace(ext, path);}
		void	setReturn(int code, std::string& path)						{ m_return.first = code; m_return.second = path;}
		void 	clearDefaultMethods()															{ m_allowedMethods.clear();}
		std::string	getPath()															const	{ return m_path; }
		void	setPath(const std::string& path)												{ m_path = path; }
		void 	setDefaultValues();
		void	printValues() const;

};