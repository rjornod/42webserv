#pragma once
#include <iostream>

class Client {
	private:
		int 				m_clientFd;
		std::string m_clientIp;
		int					m_clientPort;
		std::string	m_clientBuffer;
		bool				m_keepAlive;
	public:
		Client(int clientFd,const std::string& clientIp) :	m_clientFd(clientFd),
																												m_clientIp(clientIp),
																												m_keepAlive(true) {
			std::cout << "Client Object created\nClientFd: " << m_clientFd 
								<< "\nClient Ip: " << m_clientIp << std::endl;
		}
		~Client(){std::cout << "Client Object with fd [" << m_clientFd << "] destroyed" 
												<< std::endl;
		}
		/* getters and setters */
		const int						getClientFd() {return m_clientFd;}
		const std::string& 	getClientIp() {return m_clientIp;}
		std::string& 				getClientBuffer() {return m_clientBuffer;}
		const bool 					getKeepAlive() {return m_keepAlive;}
		void								setClientFd(int clientFd) {m_clientFd = clientFd;} 
		void								setClientIp(std::string clientIp) {m_clientIp = clientIp;}
		void								setClientBuffer(std::string clientBuffer) {m_clientBuffer = clientBuffer;}
		void								setKeepAlive(bool keepAlive) {m_keepAlive = keepAlive;}
		/* member functions*/
		void	appendToBuffer(std::string data, size_t len) {
			m_clientBuffer.append(data.c_str(), len);
    }
};