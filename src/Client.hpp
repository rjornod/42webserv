#pragma once

#include <iostream>
#include "ClientState.hpp"
#define SEND 1
#define RECEIVE 2
//client states
#define KEEP_ALIVE 7
#define CLOSING 8
#define CLOSED 9

class Client {
	private:
		int 				m_clientFd;
		std::string m_clientIp;
		int					m_clientPort;
		std::string	m_receiveBuffer;
		std::string	m_sendBuffer;
		bool				m_keepAlive;
		size_t			m_bytesSent;
		size_t			m_bytesLeftToSend;
		ClientState	m_clientState;
		time_t			m_lastActivity;							// stores the time since the client requested something
	public:
		Client(int clientFd, const std::string& clientIp) {
			m_clientFd = clientFd;
			m_clientIp = clientIp;
			m_keepAlive = true;
			m_clientState = ClientState::ReadingRequest;
			m_bytesSent = 0;
			m_lastActivity = time(nullptr);
			std::cout << "Client Object created\nClientFd: " << m_clientFd 
								<< "\nClient Ip: " << m_clientIp << std::endl;

		}
		~Client(){std::cout << "Client Object with fd [" << m_clientFd << "] destroyed" 
												<< std::endl;
		}
		/* getters and setters */
		const int						getClientFd() 																		{return m_clientFd;}
		const std::string& 	getClientIp() 																		{return m_clientIp;}
		std::string& 				getClientReceiveBuffer() 													{return m_receiveBuffer;}
		std::string& 				getClientSendBuffer() 														{return m_sendBuffer;}
		const bool 					getKeepAlive() 																		{return m_keepAlive;}
		const size_t				getBytesSent()																		{return m_bytesSent;}
		const size_t				getBytesLeftToSend()															{return m_bytesLeftToSend;}
		ClientState					getClientState() 																	{return m_clientState;}
		time_t							getLastActivity()																	{return m_lastActivity;}
		void								setClientFd(int clientFd) 												{m_clientFd = clientFd;} 
		void								setClientIp(std::string clientIp) 								{m_clientIp = clientIp;}
		void								setClientReceiveBuffer(std::string clientBuffer)	{m_receiveBuffer = clientBuffer;}
		void								setClientSendBuffer(std::string clientBuffer)			{m_sendBuffer = clientBuffer;}
		void								setKeepAlive(bool keepAlive) 											{m_keepAlive = keepAlive;}
		void								setBytesSent(size_t bytes)												{m_bytesSent = bytes;}
		void								setBytesLeftToSend(size_t bytes)									{m_bytesLeftToSend = bytes;}
		void								setClientState(ClientState state)									{m_clientState = state;}
		void 								setLastActivity()																	{m_lastActivity = time(nullptr)}
		/* other member functions*/
		void	appendToBuffer(std::string data, size_t len, int operation) {
			if (operation == RECEIVE) {
				m_receiveBuffer.append(data.c_str(), len);
			}
			else if (operation == SEND) {
				m_sendBuffer.append(data.c_str(), len);
			}
    }
};