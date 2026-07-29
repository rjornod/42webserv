#pragma once
#include <iostream>
#include <vector>
#include <map>

class LocationConfig {
	private:
		std::string 													m_path;
		std::string 													m_root;
		bool																	m_autoIndex;
		std::vector<std::string>							m_limitExcept;
		int																		m_maxBodySize;
		std::vector<std::string>							m_index;
		std::map<int, std::string>						m_errorPages;
		std::string														m_upload_store;
		std::string														m_return;
		


	public:
		LocationConfig() 														{std::cout << "LOCATION object created\n"; setDefaultValues(); };
		~LocationConfig() 													{};
		LocationConfig(const LocationConfig& other)	: m_path(other.m_path),
      m_root(other.m_root),
      m_autoIndex(other.m_autoIndex),
      m_limitExcept(other.m_limitExcept),
      m_maxBodySize(other.m_maxBodySize),
      m_index(other.m_index),
      m_errorPages(other.m_errorPages),
      m_upload_store(other.m_upload_store),
      m_return(other.m_return) { 
				// std::cout << "LocationConfig copy constructor called" << std::endl;
		}
		std::map<int, std::string>	&getErrorPages()			{ return m_errorPages;}
		void	setRoot(std::string root)											{ m_root = root;}
		void	setAutoIndex(bool isOn)												{ m_autoIndex = isOn;}
		void	setBodySize(int size)													{ m_maxBodySize = size;}
		void	setIndex(std::string index)										{ m_index.emplace_back(index);}
		void	setErrorPages(int error, std::string path)		{ m_errorPages.emplace(error, path);}
		void	setUploadStore(std::string path)							{ m_upload_store = path;}
		void	setLimitExcept(std::string method)						{ m_limitExcept.emplace_back(method);}
		void	setReturn(std::string returnType)							{ m_return = returnType;} // TO DO: maybe change the variable name
		void 	setDefaultValues() {
			m_path = "/example/path";
			m_root = "/";
			m_limitExcept.push_back("GET");
			m_limitExcept.push_back("POST");
		}
		void				setPath(std::string path)	{ m_path = path; }
		std::string	getPath()									{ return m_path; }
};

/* *
 * 
 * for a webserv project would you then add connected fds to the pollfd struct so that poll will monitor them
 	ServerConfig:

		host
		port
		server_name
		error_pages
		client_max_body_size
		a list of LocationConfig objects
	
	LocationConfig:

		path or route, like / or /images
		root
		index
		allowed_methods
		autoindex
		return / redirect info
		CGI-related settings if that location supports CGI
		upload path if you support file upload
 */