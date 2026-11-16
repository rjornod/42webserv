#pragma once

#include "IoEndpoint.hpp"
#include "ServerConfig.hpp"
#include "Client.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include <arpa/inet.h>

class Listener : public IoEndpoint {
	private:
		const ServerConfig &m_serverConfig;
		int m_port;

		void printNewClient(sockaddr_in clientAddress) {
			// TO DO: remove these debug prints
			time_t timestamp;
			time(&timestamp);
			std::cout << GREEN << "New client connected: " << RESET << inet_ntoa(clientAddress.sin_addr) << ":" 
			<< ntohs(clientAddress.sin_port) << std::endl;
			std::cout << ctime(&timestamp) << std::endl;					// displays the exact time a client connected	
		}
	public:
		Listener(int fd, int port, const ServerConfig &serverConfig) : IoEndpoint(fd), m_serverConfig(serverConfig) {
			m_port = port;
			std::cout << "Listener Object created\nFd: " << m_fd 
								<< "\nPort: " << m_port << std::endl;

		}

		EndpointResult processEvent(IoEvent ev) noexcept override {
			try {
				if (ev.isError || ev.isWrite) {
					return EndpointResult::ERROR;
				}
				while (true) {
					sockaddr_in clientAddress;		// struct holding IPv4 info about the client
					socklen_t clientAddressLen = sizeof(clientAddress);	// containts the length of the client socket
					int clientFd = accept(m_fd, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressLen);
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

					printNewClient(clientAddress);

					spawn(std::make_unique<Client>(clientFd, inet_ntoa(clientAddress.sin_addr), m_port, m_serverConfig));
				}
			} catch (std::exception &ex) {
				(void)ex; // todo print ex
				return EndpointResult::ERROR;
			}
			if (m_spawned.empty()) {
				return EndpointResult::EMPTY;
			}
			return EndpointResult::SPAWNED;
		}

		bool validate() override {
			return true; // no concern with server fringe cases
		}
};
