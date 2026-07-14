#pragma once
#include <iostream>
#include <vector>

class LocationConfig {
	private:
		std::string 							m_path;
		std::string 							m_root;
		std::vector<std::string>	m_allowedMethods;

	public:
		LocationConfig() {setDefaultValues(); };
		~LocationConfig() {};
		LocationConfig(const LocationConfig& other){std::cout << "LocationConfig copy constructor called" << std::endl;}
		void setDefaultValues() {
			m_path = "/example/path";
			m_root = "/";
			m_allowedMethods.push_back("GET");
			m_allowedMethods.push_back("POST");
		}
};

/**
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