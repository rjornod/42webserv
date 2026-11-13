#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <ctime>			// ctime
#include <sstream>		//ostringstream
#include <algorithm>	// std::find
#include <csignal>		//signal()
#include "Server.hpp"
#include "Client.hpp"
#include "Colors.hpp"
#include "HttpParser.hpp"
#include "Router.hpp"
#include "RequestContext.hpp"
#include "RequestProcessor.hpp"
#include "HttpResponse.hpp"
#include "ResponseWriter.hpp"
#include "Listener.hpp"
#include <fcntl.h>

volatile sig_atomic_t serverRunning = true;

void	Server:: closeAllFds() {
	std::cout << "closing all fds" << std::endl;

	// close the fds of the connected clients and listening sockets
	for (unsigned long i = 0; i < m_connectedFds.size(); i++)
		close(m_connectedFds[i].fd);
}

void signalHandler(int sig) {
	(void)sig;
	std::cout << "Program interrupted by SIGINT" << std::endl;
	serverRunning = false;
}

int Server::serverSetup() {
	for (unsigned long i = 0; i < m_config.getServerConfigs().size(); i++) {
		const ServerConfig& serverConfig = m_config.getServerConfigs()[i];

		sockaddr_in tcpAddress;
		std::memset(&tcpAddress, 0, sizeof(tcpAddress));
		tcpAddress.sin_family = AF_INET; 					// dictates a socket will use ipv4 addressing
		tcpAddress.sin_addr.s_addr = INADDR_ANY; 	// allows a server to accept traffic across any local ip address
		tcpAddress.sin_port = htons(serverConfig.getPort());
		int fd = socket(AF_INET, SOCK_STREAM, 0); 			// creates the fd for the listening socket, comes back blocking by default
		if (fd < 0) {
			perror("socket"); 
			return 1;
		}
		int flags = fcntl(fd, F_GETFL);
		if (flags < 0) {
			perror("fcntl error");
			return 1;
		}
		/* sets the fd to non blocking meaning accept won't block if there are currently no clients waiting to connect */
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) { 
			perror("fcntl F_SETFL error");
			return 1;
		}
		int enable = 1; 																														// once a tcp socket closes, the port is only free after around 2 minutes (TIME_WAIT state).
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) 	// setsockopt with the SO_REUSEADDR tells the kernel to reuse the socket as long as no active connection is using it
			std::cout << ("setsockopt(SO_REUSEADDR) failed") << std::endl;
	
		int returnValue;
		returnValue = bind(fd, reinterpret_cast<const sockaddr *>(&tcpAddress), sizeof(tcpAddress)); // bind associates the socket with a specific local address and port on the machine
		if (returnValue < 0) {
			perror("bind");
			close(fd);
			return 1;
		}
		/**
		 * marks the socket as one that will be used to accept incoming requests
		 * backlog parameter sets the length of the queue of pending connections to 128
		 */
		returnValue = listen(fd, 128);
		if (returnValue < 0) {
			perror("listen");
			return 1;
		}
		m_endpoints[fd] = std::make_unique<Listener>(fd, serverConfig.getPort(), serverConfig);
		m_poller->registerFd(fd); // add listening socket to the poll watch list
	}
	std::cout << BLUE << "\n[ SERVER IS LISTENING ]" << RESET << std::endl;
	return 0;
}

void Server::eraseClient(int fd) {
	m_poller->unregisterFd(fd);
	m_endpoints.erase(fd);
}

int Server::serverCore() {
	signal(SIGINT, signalHandler);
	while (serverRunning) {
		auto events = m_poller->doPoll();
		for (auto ev : events) {
			auto processRes = m_endpoints[ev.fd]->processEvent(ev);
			if (processRes == IoEndpoint::EndpointResult::ERROR) {
				eraseClient(ev.fd);
			} else if (processRes == IoEndpoint::EndpointResult::SPAWNED) {
				for (auto &&ep : m_endpoints[ev.fd]->collectSpawned()) {
					int fd = ep->getFd();
					m_endpoints[fd] = std::move(ep);
					m_poller->registerFd(fd);
				}
			}
		}
	}
	return 0;
}
