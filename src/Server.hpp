#pragma once

#include <iostream>
#include <vector>
#include <map>
#include "GlobalConfig.hpp"
#include "Client.hpp"
#include <poll.h>
#include <cstring>
#include <netinet/in.h> //sockaddr_in
#include <fcntl.h> 			// fcntl
#include <unistd.h>

class Server {
	private:
		const	GlobalConfig& 				m_config; 					// const reference to the parsed config file
		std::vector<int> 						m_serverFds; 				// vector holding the fds of each server in the config file
		std::vector<struct pollfd>	m_connectedFds;			// vector holding all the pollfd structs
		std::map<int, Client> 			m_connectedClients;	// map holding the fd and object of each connected client
		struct sockaddr_in 					m_tcpAddress;				// struct holding IP_v4 address information, used to specify where to connect, bind or listen for traffic 
		socklen_t 									m_tcpAddressLen;		// contains the length of the socket
		int 												m_tcpServerFd;			// fd that will be used for the listening socket of the server
		std::string 								m_buffer;						// stores the messages recieved from the clients
		struct sockaddr_in					m_clientAddress;		// struct holding IPv4 info about the client
		socklen_t										m_clientAddressLen;	// containts the length of the client socket
	public:
		Server(const GlobalConfig& parsedConfig) : m_config(parsedConfig),
			m_tcpAddressLen(sizeof(sockaddr_in)), m_clientAddressLen(sizeof(m_clientAddress)) {
			std::cout << "Server object Created" << std::endl;
		}
		~Server() {
			std::cout << "Server object destroyed" << std::endl;
			closeAllFds();
		}
		const GlobalConfig& getParsedConfig() {return m_config;}
		int 								setTcpAddress() 	{
			std::memset(&m_tcpAddress, 0, sizeof(sockaddr_in));
			m_tcpAddress.sin_family = AF_INET; 					// dictates a socket will use ipv4 addressing
			m_tcpAddress.sin_addr.s_addr = INADDR_ANY; 	// allows a server to accept traffic across any local ip address
			return 0;
		}
		const std::vector<pollfd> &getConnectedFds() const {return m_connectedFds;}
		int 				serverSetup();
		int 				connections();
		int 				serverCore();
		bool 				recieveRequest(Client& client);
		bool				buildResponse(Client& client);
		bool 				sendResponse(Client& client);
		void				eraseClient(int fd);
		void				closeAllFds();
		bool				readFile(Client& client);
};