#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <ctime>			// ctime
#include <sstream>		//ostringstream
#include <algorithm>	// std::find
#include <csignal>		//signal()
#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Colors.hpp"

volatile sig_atomic_t serverRunning = true;

void	Server:: closeAllFds() {
	std::cout << "closing all fds" <<std::endl;
	for (int i = 0; i < m_connectedFds.size(); i++) {
		close(m_connectedFds[i].fd);
	}
}

void signalHandler(int sig) {
	std::cout << "Program interrupted by SIGINT" <<std::endl;
	serverRunning = false;
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
	/* sets the fd to non blocking meaning accept won't block if there are currently no clients waiting to connect */
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
	std::cout << BLUE << "[ SERVER IS LISTENING ]" << RESET << std::endl;
	return 0;
}

void Server::receiveRequest(Client& client) {
	ssize_t	bytes;
	char		buffer[4096];
	// TO DO: check for errors in recv (-1 and 0) and remove client on error
	bytes = recv(client.getClientFd(), buffer, sizeof(buffer), 0);
	if (bytes > 0) {
		client.setLastActivity();
		client.setClientState(ClientState::ReadingRequest);
		client.appendToBuffer(buffer, bytes, RECEIVE); // the data appended to the buffer here will be the request
		/**
		 ** The buffer should keep being appended to until the request is complete.
		 ** I guess the HTTP parsing would go in here.
		 ** Once the requested is confirmed to be complete, it should mark the socket for POLLOUT
		 * which means we are telling the kernel that we want to be notified as soon as the socket
		 * is ready to receive data
		 ** The response should then be built on a new string (?) to then be sent to the client
		 * 
		 **/
		client.setClientState(ClientState::RequestFinished);
		// this loop should only happen AFTER the full request has come in
		buildResponse(client);																// check if build response gave an error
		std::cout << "-------------------" << MAGENTA << " REQUEST FROM: FD " <<  client.getClientFd() << RESET << "-------------------" << std::endl;
		std::cout << client.getClientReceiveBuffer() << MAGENTA << "------------------"<< " END OF REQUEST " << "--------------------" << RESET << std::endl;
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
		std::cout << client.getClientIp() << RED << " disconnected at:\n" << RESET << ctime(&timestamp) <<  std::endl;
		client.setShouldDisconnect(true);
	}
	if (bytes < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)  // when a socket is set to non blocking, operations that would wait instead return one of these error immediately. Ignore for non blocking behavior
			client.setShouldDisconnect(false);
		perror("recv");
		client.setShouldDisconnect(true);
	}
}

int Server::connections() {
	m_connectedFds.emplace_back(pollfd{m_tcpServerFd, POLLIN, 0}); // add the listening socket fd to the poll list 
	m_connectedFds[0].events = (POLLIN);
	return 0;
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
	std::cout << RED << "Clients connected: " << RESET << m_connectedClients.size() << std::endl;
}

bool Server::readFile(Client& client) {
	// std::string string;
	int fileFd = open("data/www/index.html", O_RDONLY);
	if (fileFd < 0) {
		std::cout << "error" << std::endl;
		return true;
	}
	char buffer[100];
	ssize_t bytes;

	while ((bytes = read(fileFd, buffer, sizeof(buffer))) > 0)
	{
		if (bytes > 0) 
			client.getClientSendBuffer().append(buffer, bytes);
		else if (bytes == 0)
			break;
		else if (bytes == -1)
			return true;
	}
	close(fileFd);
	return false;
}

/*
	TO DO: figure out if there should be a timeout to disconnect users after a set time
*/
void Server::checkTimeouts() {
	for (auto it = m_connectedClients.begin(); it != m_connectedClients.end(); ++it) {
		if (time(nullptr) - it->second.getLastActivity() > 60
			 && it->second.getClientState() == ClientState::ReadingRequest) {
					// it->second.setShouldDisconnect(true);
		}
	}
}

void Server::buildResponse(Client& client) {
	std::string headers;
	client.setClientState(ClientState::BuildingResponse);
	if (readFile(client)) {																								// check if readFile returned an error
		std::cout << "readfile error" << std::endl;
		client.setShouldDisconnect(true);
	}
	headers += "HTTP/1.1 200 OK\r\n";
	headers += "Content-Type: text/html\r\n";
	headers += "Content-Length: " + std::to_string(client.getClientSendBuffer().size()) + "\r\n";
	headers += "Connection: keep-alive\r\n";
	headers += "\r\n";
	client.getClientSendBuffer().insert(0, headers);
	client.setShouldDisconnect(false);
}

void Server::sendResponse(Client& client) {
	size_t 	sentBytes = 0;
	client.setBytesLeftToSend(client.getClientSendBuffer().size() - client.getBytesSent());					// calculates the remaining bytes we need to send
	client.setClientState(ClientState::SendingResponse);
	sentBytes = send(client.getClientFd(), client.getClientSendBuffer().c_str() + client.getBytesSent(), client.getBytesLeftToSend(), 0); 	// makes sure to only send the data we havent sent (if spread among multiple calls)
	if (sentBytes > 0) {
		 client.setBytesSent(client.getBytesSent() + sentBytes);
		 if (client.getBytesSent() == client.getClientSendBuffer().size()) {
			client.getClientSendBuffer().clear();																// sendBuffer gets cleared if we finished sending everything
			client.getClientReceiveBuffer().clear();														// receiveBuffer also gets cleared
			client.setBytesSent(0);																							// bytesSent gets reset if we finished sending everything
			client.setClientState(ClientState::ReadingRequest);									// reset ClientState back to readingRequest after response is sent
			// client.setLastActivity(); 																				// TO DO: figure out if we need to update the timestamp here
		}
	}
	else if (sentBytes < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {												// if there is no more data to send at the moment, we ignore these errors
			client.setShouldDisconnect(false);   
		}
		else {
			perror("send()");
			client.setShouldDisconnect(true);
		}
	}
	client.setShouldDisconnect(false);
}

bool	Server::handleIncoming(int fd) {
	auto it = m_connectedClients.find(fd);
	if (it != m_connectedClients.end()) {
		receiveRequest(it->second);
		if (it->second.getShouldDisconnect() == true) {																							// it->second passes the client object to receive. it->first would pass the fd 
			eraseClient(fd);
			return true;																																							// client got disconnected
		}																						
	}
	return false;																																									// client is still connected
}

bool	Server::handleOutgoing(size_t i) {
	auto it = m_connectedClients.find(m_connectedFds[i].fd);
	if (it != m_connectedClients.end()) {
		sendResponse(it->second);
		if (it->second.getShouldDisconnect() == true) {
			eraseClient(m_connectedFds[i].fd);
			return true;																																	// client got disconnected
		}
		if (it->second.getClientSendBuffer().empty()){																	// if we finished sending our response we add POLLIN to events to again listen for data being sent
			m_connectedFds[i].events = POLLIN;	
		}				
		else 
			m_connectedFds[i].events = POLLOUT;																						// if we didnt finish sending everything, keep POLLOUT in events to keep sending data
		}
		return false;																																		// client is still connected
}

int Server::serverCore() {
	signal(SIGINT, signalHandler);
	while (serverRunning) {
		int clientFd;
		int returnValue = poll(m_connectedFds.data(), m_connectedFds.size(), 0); // 0 timeout makes poll constantly check all the clients
		if (returnValue < 0) {
			if (errno == EINTR) {
						continue;
			}
			perror("poll");
			return 1;
		}
		// checkTimeouts();
		if (m_connectedFds[0].revents & POLLIN) {
			while (true) {
				clientFd = accept(m_tcpServerFd, 
									 reinterpret_cast<sockaddr *>(&m_clientAddress), &m_clientAddressLen);
				if (clientFd < 0) {
					if (errno == EAGAIN || errno == EWOULDBLOCK)						// when there are no more clients to accept() it returns -1 with these errnos. they are not real errors so we break to keep checking for clients without failure
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
				m_connectedFds.push_back(pollfd{clientFd, POLLIN, 0}); 		// creates a new pollfd struct initialized with the clientFd and inserts it in the poll struct
				time_t timestamp;
				time(&timestamp);
				std::cout << GREEN << "New client connected: " << RESET << inet_ntoa(m_clientAddress.sin_addr) << ":" 
				<< ntohs(m_clientAddress.sin_port) << std::endl;
				std::cout << ctime(&timestamp) << std::endl;					// displays the exact time a client connected											
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
 				if (handleIncoming(m_connectedFds[i].fd)) {
					i--;
					continue;
				} 																									
			}
			if (m_connectedFds[i].revents & POLLOUT) {
				if (handleOutgoing(i)) {																													// if client was removed decrease the index
					i--;
					continue;
				}
			}
		}
	}
	return 0;
}
