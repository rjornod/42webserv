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
		LocationConfig() 																											{ setDefaultValues(); };
		~LocationConfig() 																										{};
		void	setRoot(const std::string& root)																{ m_root = root; }
		void	setAutoIndex(bool isOn)																					{ m_autoIndex = isOn; }
		void	setBodySize(int size)																						{ m_maxBodySize = size; }
		void	setIndex(const std::string& index)															{ m_index.emplace_back(index); }
		void	setErrorPages(int error, const std::string& path)								{ m_errorPages.emplace(error, path); }
		void	setUploadStore(const std::string& path)													{ m_upload_store = path; }
		void	setAllowedMethod(const std::string& method)											{ m_allowedMethods.emplace_back(method); }
		void 	setCgiHandler(const std::string& ext, const std::string& path)	{ m_cgiHandlers.try_emplace(ext, path); }
		void	setReturn(int code, const std::string& path)										{ m_return.first = code; m_return.second = path; }
		void	setPath(const std::string& path)																{ m_path = path; }
		void 	clearDefaultMethods()																						{ m_allowedMethods.clear(); }
		void 	setDefaultValues();
		const std::string& 									getPath() const												{ return m_path; }
		bool 																getAutoIndex() const 									{ return m_autoIndex; } 
		const std::string& 									getRoot() const 											{ return m_root; }
		int 																getMaxBodySize() const								{ return m_maxBodySize; }
		const std::vector<std::string>& 		getIndex() const 											{ return m_index; }
		const std::map<int, std::string>&		getErrorPages() const									{ return m_errorPages; }
		const std::vector<std::string>& 		getAllowedMethods() const							{ return m_allowedMethods; }
		const std::string& 									getUploadStore() const 								{ return m_upload_store; }
		const std::pair<int, std::string>& 	getReturn() const											{ return m_return; }
		const	std::unordered_map<std::string, std::string>& getCgiHandler() const	{ return m_cgiHandlers; }
		
			const std::string& 								getPath()												{ return m_path; }
		bool 																getAutoIndex() 									{ return m_autoIndex; } 
		const std::string& 									getRoot() 											{ return m_root; }
		int 																getMaxBodySize()								{ return m_maxBodySize; }
		const std::vector<std::string>& 		getIndex() 											{ return m_index; }
		const std::map<int, std::string>&		getErrorPages()									{ return m_errorPages; }
		const std::vector<std::string>& 		getAllowedMethods()							{ return m_allowedMethods; }
		const std::string& 									getUploadStore() 								{ return m_upload_store; }
		const std::pair<int, std::string>& 	getReturn()											{ return m_return; }
		const	std::unordered_map<std::string, std::string>& getCgiHandler()	{ return m_cgiHandlers; }
		void	printValues() const;

};