#pragma once

#include <iostream>
#include <vector>
#include <map>
#include "GlobalConfig.hpp"
#include "Client.hpp"
#include "Polling.hpp"
#include <poll.h>
#include <cstring>
#include <netinet/in.h> //sockaddr_in

struct ListeningSockets {
		int fd;
		int port;
		std::vector<size_t> serverIndexes;
	};
	
class Server {
	private:
		const	GlobalConfig& 						m_config; 					// const reference to the parsed config file
		std::unique_ptr<Poller> m_poller; // the poller
		std::unordered_map<int, IoEndpointBox> m_endpoints; // all the endpoints
		std::vector<struct pollfd>			m_connectedFds;			// vector holding all the pollfd structs
		std::map<int, Client> 					m_connectedClients;	// map holding the fd and object of each connected client
		struct sockaddr_in 							m_tcpAddress;				// struct holding IP_v4 address information, used to specify where to connect, bind or listen for traffic 
		// socklen_t 											m_tcpAddressLen;		// contains the length of the socket
		std::vector<ListeningSockets> 	m_listeners;
		std::string 										m_buffer;						// stores the messages received from the clients
		
	public:
		Server(const GlobalConfig& parsedConfig) : m_config(parsedConfig), m_poller(createPoller()) {}
		~Server() {
			std::cout << "Server object destroyed" << std::endl;
		}
		const GlobalConfig& getParsedConfig() const {return m_config;}
		int 								setTcpAddress() 	{
			std::memset(&m_tcpAddress, 0, sizeof(sockaddr_in));
			m_tcpAddress.sin_family = AF_INET; 					// dictates a socket will use ipv4 addressing
			m_tcpAddress.sin_addr.s_addr = INADDR_ANY; 	// allows a server to accept traffic across any local ip address
			return 0;
		}

		int 		serverSetup();
		int 		serverCore();
		void 		receiveRequest(Client& client);
		void		buildResponse(Client& client);
		void 		sendResponse(Client& client);
		void		eraseClient(int fd);
		void		closeAllFds();
		void		checkTimeouts();
		bool		handleIncoming(int fd);
		bool		handleOutgoing(size_t i);
		void 		checkFdEvents();

		// debug
		void	printNewClient();
	
		
};
