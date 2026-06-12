#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <ctime>			// ctime
#include <sstream>		//ostringstream
#include <iterator>		// distance()
#include <algorithm>	// std::find
#include <csignal>		//signal()
#include "Server.hpp"
#include "Client.hpp"

volatile sig_atomic_t serverRunning = true;

void	Server::closeAllFds() {
	std::cout << "closing all fds" <<std::endl;
	for (int i = 0; i < m_connectedFds.size(); i++) {
		close(m_connectedFds[i].fd);
	}
}

// TO DO: Handle signals to not leak fds or memory
void signalHandler(int sig) {
	std::cout << "Program interrupted by SIGINT" <<std::endl;
	serverRunning = false;
	std::cout << "ServerRunning: " << serverRunning << std::endl;
}

int Server::serverSetup() {
	setTcpAddress();
	m_tcpAddress.sin_port = htons(m_config.getServerConfigs()[0].getPort());
	m_tcpServerFd = socket(AF_INET, SOCK_STREAM, 0); 			// creates the fd for the listening socket, comes back blocking by default
	if (m_tcpServerFd < 0) {
		perror("socket"); 
		return 1;
	}
	int flags = fcntl(m_tcpServerFd, F_GETFL);
	if (flags < 0) {
		perror("fcntl error");
		return 1;
	}
	/* sets the fd to non blocking meaning accept wont block if there are currently no clients waiting to connect */
	if (fcntl(m_tcpServerFd, F_SETFL, flags | O_NONBLOCK) < 0) { 
		perror("fcntl F_SETFL error");
		return 1;
	}
	int enable = 1; 																																				// once a tcp socket closes, the port is only free after around 2 minutes (TIME_WAIT state).
	if (setsockopt(m_tcpServerFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) 	// setsockopt with the SO_REUSEADDR tells the kernel to reuse the socket as long as no active connection is using it
    std::cout << ("setsockopt(SO_REUSEADDR) failed") << std::endl;

	int returnValue;
	returnValue = bind(m_tcpServerFd, reinterpret_cast<const sockaddr *>(&m_tcpAddress), sizeof(m_tcpAddress)); // bind associates the socket with a specific local address and port on the machine
	if (returnValue < 0) {
		perror("bind");
		close(m_tcpServerFd);
		return 1;
	}
	/**
	 * marks the socket as one that will be used to accept incoming requests
	 * backlog parameter sets the length of the queue of pending connections to 128
	 */
	returnValue = listen(m_tcpServerFd, 128);
	if (returnValue < 0) {
		perror("listen");
		return 1;
	}
	std::cout << "[ SERVER IS LISTENING ]" << std::endl;

	return 0;
}

void Server::recieveRequest(Client& client) {
	ssize_t	bytes;
	char		buffer[4096];
	bytes = recv(client.getClientFd(), buffer, sizeof(buffer), 0);
	if (bytes > 0) {
		client.appendToBuffer(buffer, bytes); // the data appended to the buffer here will be the request
		/**
		 ** The buffer should keep being appended to until the request is complete.
		 ** I guess the HTTP parsing would go in here.
		 ** Once the requested is confirmed to be complete, it should mark the socket for POLLOUT
		 * which means we are telling the kernel that we want to be notified as soon as the socket
		 * is ready to recieve data
		 ** The response should then be built on a new string (?) to then be sent to the client
		 * 
		 **/

		// this loop should only happen AFTER the full request has come in
		for (int i = 1; i < m_connectedFds.size(); i++) {					// loop that goes through every member of the pollfd struct 
			if (m_connectedFds[i].fd == client.getClientFd()) {	
				m_connectedFds[i].events |= POLLOUT; 									// |= bitwise OR operator, adds POLLOUT to the list of flags to watch out for
				break;
			}
		}
	}
	if (bytes == 0) {
		time_t timestamp;
		time(&timestamp);
		std::cout << ctime(&timestamp);																			// displays the exact time a client disconnected
		std::cout << client.getClientIp() << " disconnected\n" << std::endl;
		eraseClient(client.getClientFd());
		return;
	}
	if (bytes < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)  // when a socket is set to non blocking, operations that would wait instead return one of these error immediately. Ignore for non blocking behavior
			return;
		perror("recv");
		eraseClient(client.getClientFd());
		return;
	}
}

int Server::connections() {
	m_connectedFds.emplace_back(pollfd{m_tcpServerFd, POLLIN, 0}); // add the listening socket fd to the poll list 
	m_connectedFds[0].events = (POLLIN);
	return 0;
}

void debugMessage(std::string msg) {
	std::cout << msg << std::endl;
}

void Server::eraseClient(int fd) {
	m_connectedClients.erase(fd); 										// deletes the disconnected client from the list of connected clients
	close(fd);																				// closes the fd
	for (int i = 1; i < m_connectedFds.size(); i++) {	// loop that goes through every member of the pollfd struct 
		if (m_connectedFds[i].fd == fd) {								// if the fd member variable is the same as the fd we passed, we found the element we want to erase
			m_connectedFds[i] = m_connectedFds.back();		// swaps the disconnected element with the last one in the vector
			m_connectedFds.pop_back();										// erases the last element in the vector, which is now the disconnected client
			break ; 																			// break after removal to not keep scanning the vector
		}
	}							
}

std::string readFile() {
	int fileFd = open("www/index.html", O_RDONLY);
	if (fileFd < 0) {
		std::cout << "error" << std::endl;
	}
	char buffer[1086];
	ssize_t bytes = read(fileFd, buffer, sizeof(buffer));
	std::cout << buffer << std::endl;
	close(fileFd);
	std::string string = buffer;
	return string;
}

void Server::sendResponse(Client& client) {
	// std::string body = "This is the message you requested!";
	std::string body = readFile();
	std::cout << client.getClientBuffer() << std::endl;
	std::string response;
	response += "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
	response += "Connection: keep-alive\r\n";
	response += "\r\n";
	response += body;
	int sentBytes = 0;
	sentBytes = send(client.getClientFd(), response.c_str(), response.size(), 0);
	if (sentBytes > 0) {
		 std::cout << "Sent message successfully" << std::endl;
	}
	else if (sentBytes < 0) {
		perror("send()");
		return ;
	}
	client.getClientBuffer().clear();
}

int Server::serverCore() {
	while (serverRunning) {
		signal(SIGINT, signalHandler);
		int clientFd;
		int returnValue = poll(m_connectedFds.data(), m_connectedFds.size(), -1); // timeout set to -1 for an infinite timeout
		if (returnValue < 0) {
			perror("poll");
			return 1;
		}
		if (m_connectedFds[0].revents & POLLIN) {
			while (true) {
				// m_clientAddressLen = sizeof(m_clientAddress); // done in constructor
				clientFd = accept(m_tcpServerFd, 
									 reinterpret_cast<sockaddr *>(&m_clientAddress), &m_clientAddressLen);
				if (clientFd < 0) {
					if (errno == EAGAIN || errno == EWOULDBLOCK)						// when there is no more clients to accept() it returns -1 with these errnos. they are not real errors so we break to keep checking for clients without failure
						break;
					perror("accept");
					break;
				}
				int clientFlags = fcntl(clientFd, F_GETFL, 0);
				if (clientFlags < 0) {
						perror("fcntl client F_GETFL");
						close(clientFd);
						continue;
				}
				if (fcntl(clientFd, F_SETFL, clientFlags | O_NONBLOCK) < 0) {
						perror("fcntl client F_SETFL");
						close(clientFd);
						continue;
				}
				m_connectedFds.push_back(pollfd{clientFd, POLLIN, 0});																		// creates a new pollfd struct initialized with the clientFd and inserts it in the poll struct
				std::cout << "Client connected " << inet_ntoa(m_clientAddress.sin_addr) << ":" 
				<< ntohs(m_clientAddress.sin_port) << std::endl;
				/* creates a client object directly on m_connectedClients and initializes the fd and ip address */
				m_connectedClients.try_emplace(clientFd, clientFd, inet_ntoa(m_clientAddress.sin_addr));	// only inserts if clientFd is not present. clientfd is the map key, clientFd and inet_ntoa() are sent to the Client constructor
			}
		}
		/* handle incoming data from the connected clients */
		for (size_t i = 1; i < m_connectedFds.size(); i++) {
			if (m_connectedFds[i].revents & POLLHUP || m_connectedFds[i].revents & POLLERR) { 	// errors related to client disconnect
				eraseClient(m_connectedFds[i].fd);
				i--;																																							// decrement i to account for the client deletion
				continue;																																					// continue so we dont check for POLLIN and POLLOUT for the deleted client
			}
			if (m_connectedFds[i].revents & POLLIN) { 																					// socket has fresh data for us
 				auto it = m_connectedClients.find(m_connectedFds[i].fd);
        if (it != m_connectedClients.end()) {
            recieveRequest(it->second); 																									// it->second passes the client object to recieve. it->first would pass the fd 
        }
			}
			if (m_connectedFds[i].revents & POLLOUT) { 																					// the socket is ready to recieve data
				auto it = m_connectedClients.find(m_connectedFds[i].fd);
        if (it != m_connectedClients.end()) {
					sendResponse(it->second);
					m_connectedFds[i].events = POLLIN;																							// TO DO: should only be done after the full send-buffer is cleared. resets the events to lookout for to POLLIN
				}
			}
			if (!serverRunning)
				closeAllFds();
		}
	}
	return 0;
}
