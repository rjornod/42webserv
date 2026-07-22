#pragma once
#include <iostream>
#include <vector>

class LocationConfig {
	private:
		std::string 							m_path;
		std::string 							m_root;
		bool											m_autoIndex;
		std::vector<std::string>	m_limitExcept;
		int												m_maxBodySize;
		std::string								m_index;
		std::vector<std::string>	m_errorPages;
		std::string								m_upload_store;
		std::string								m_return;
		


	public:
		LocationConfig() 														{setDefaultValues(); std::cout << "location config object created\n"; };
		~LocationConfig() 													{};
		LocationConfig(const LocationConfig& other)	{std::cout << "LocationConfig copy constructor called" << std::endl;}
		void setDefaultValues() {
			m_path = "/example/path";
			m_root = "/";
			m_limitExcept.push_back("GET");
			m_limitExcept.push_back("POST");
		}
		void 				setPath(std::string path)	{ m_path = path; }
		std::string getPath()									{ return m_path; }
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