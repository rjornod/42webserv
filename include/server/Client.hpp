#pragma once

#include <iostream>
#include "ClientState.hpp"
#include "IoEndpoint.hpp"
#include "ServerConfig.hpp"
#define SEND 1
#define RECEIVE 2
#include "Colors.hpp"
#include "HttpParser.hpp"
#include "HttpResponse.hpp"
#include "ResponseWriter.hpp"
//client states
#define KEEP_ALIVE 7
#define CLOSING 8
#define CLOSED 9


class Client : public IoEndpoint {
	private:
		std::string m_clientIp;
		int					m_localPort;
		std::string	m_receiveBuffer;
		std::string	m_sendBuffer;
		bool				m_keepAlive;
		size_t			m_bytesSent;
		size_t			m_bytesLeftToSend;
		ClientState	m_clientState;
		time_t			m_lastActivity;							// stores the time since the client requested something
		bool				m_shouldDisconnect;					// marks a client for disconnection
		const ServerConfig &m_serverConfig;
		HttpParser 	m_parser;
		std::unique_ptr<ResponseWriter> m_writer; //A pointer that will be null until the response is ready

	public:
		Client(int clientFd, const std::string& clientIp, int localPort, const ServerConfig &serverConfig) : IoEndpoint(clientFd), m_serverConfig(serverConfig) {
			m_clientIp = clientIp;
			m_localPort = localPort;
			m_keepAlive = true;
			m_clientState = ClientState::ReadingRequest;
			m_bytesSent = 0;
			m_lastActivity = time(nullptr);
			m_shouldDisconnect = false;
			std::cout << "Client Object created\nClientFd: " << m_fd 
								<< "\nClient Ip: " << m_clientIp << std::endl;

		}
		~Client(){std::cout << "Client Object with fd [" << m_fd << "] destroyed" 
												<< std::endl;
		}
		/* getters and setters */
		HttpParser&					getParser()																				{return m_parser;}		const std::string& 	getClientIp() 																		{return m_clientIp;}
		int 								getLocalPort()																		{return m_localPort;}
		std::string& 				getClientReceiveBuffer() 													{return m_receiveBuffer;}
		std::string& 				getClientSendBuffer() 														{return m_sendBuffer;}
		bool 								getKeepAlive() 																		{return m_keepAlive;}
		size_t							getBytesSent()																		{return m_bytesSent;}
		size_t							getBytesLeftToSend()															{return m_bytesLeftToSend;}
		ClientState					getClientState() 																	{return m_clientState;}
		time_t							getLastActivity()																	{return m_lastActivity;}
		bool								getShouldDisconnect()															{return m_shouldDisconnect;}
		void								setClientIp(std::string clientIp) 								{m_clientIp = clientIp;}
		void								setLocalPort(int localPort)												{m_localPort = localPort;}
		void								setClientReceiveBuffer(std::string clientBuffer)	{m_receiveBuffer = clientBuffer;}
		void								setClientSendBuffer(std::string clientBuffer)			{m_sendBuffer = clientBuffer;}
		void								setKeepAlive(bool keepAlive) 											{m_keepAlive = keepAlive;}
		void								setBytesSent(size_t bytes)												{m_bytesSent = bytes;}
		void								setBytesLeftToSend(size_t bytes)									{m_bytesLeftToSend = bytes;}
		void								setClientState(ClientState state)									{m_clientState = state;}
		void 								setLastActivity()																	{m_lastActivity = time(nullptr);}
		void								setShouldDisconnect(bool shouldDisconnect) 				{m_shouldDisconnect = shouldDisconnect;}		
		/* other member functions*/
		void	appendToBuffer(std::string data, size_t len, int operation) {
			if (operation == RECEIVE) {
				m_receiveBuffer.append(data.c_str(), len);
			}
			else if (operation == SEND) {
				m_sendBuffer.append(data.c_str(), len);
			}
    }

		void writerSetup(HttpResponse &&response) {
			m_writer = std::make_unique<ResponseWriter>(std::move(response));
		}

		bool responseReady() const {return m_writer != nullptr;}

		bool writerFinished() const {return !m_writer || m_writer->getState() == WriterState::DONE;}

		// bool writerError(const WriteResult& result) const {return result == WriteResult::ERROR;}

		void writeToSocket() {m_writer->writeTo(m_fd);}

		void resetWriter() {m_writer = nullptr;}

		void clearParser() {m_parser.clearParser();}

		std::string printState() {
			std::cout << "Client fd: " << m_fd << " - ";
			if (m_clientState == ClientState::ReadingRequest)
				return "ReadingRequest";
			else if (m_clientState == ClientState::RequestFinished)
				return "RequestFinished";
			else if (m_clientState == ClientState::BuildingResponse)
				return "BuildingResponse";
			else if (m_clientState == ClientState::SendingResponse)
				return "SendingResponse";
			else
				return "error";
		};

		void receiveRequest() {
			HttpParser&	parser = getParser();
			if (parser.getParserState() == HttpParserState::COMPLETE ||
					parser.getParserState() == HttpParserState::ERROR) {
				return; // ignore after the first request
				// TODO: expand it for the CGI case
			}
			ssize_t	bytes;
			char		buffer[4096];
			// TO DO: check for errors in recv (-1 and 0) and remove client on error

			bytes = recv(m_fd, buffer, sizeof(buffer), 0);
			if (bytes > 0) {
				parser.parse(buffer);
					
				setLastActivity();
				setClientState(ClientState::ReadingRequest);
			}

			/**
				** The buffer should keep being appended to until the request is complete.
				** I guess the HTTP parsing would go in here.
				** Once the requested is confirmed to be complete, it should mark the socket for POLLOUT
				* which means we are telling the kernel that we want to be notified as soon as the socket
				* is ready to receive data
				** The response should then be built on a new string (?) to then be sent to the client
				* 
			**/
			if (parser.getParserState() == HttpParserState::COMPLETE ||
					parser.getParserState() == HttpParserState::ERROR) {
				setClientState(ClientState::RequestFinished);

				// this loop should only happen AFTER the full request has come in
				if (parser.getParserState() == HttpParserState::COMPLETE)
					buildResponse();																// check if build response gave an error
				else
					buildParserErrorResponse();
			}
			std::cout << "-------------------" << MAGENTA << " REQUEST FROM: FD " <<  m_fd << RESET << "-------------------" << std::endl;
			// std::cout << "-------------------" << MAGENTA << "connection through index: " << getServerConfigIndex() << " of the global config " << RESET << "-------------------" << std::endl;
			std::cout << getParser().getRequest() << std::endl;
			std::cout << getClientReceiveBuffer() << MAGENTA << "------------------"<< " END OF REQUEST " << "--------------------" << RESET << std::endl;	
					
					
			if (bytes == 0) {
				time_t timestamp;
				time(&timestamp);
				std::cout << getClientIp() << RED << " disconnected at:\n" << RESET << ctime(&timestamp) <<  std::endl;
				setShouldDisconnect(true);
			}

			if (bytes < 0) {
				// if -1 is returned just disconnect the client
				setShouldDisconnect(true);
			}
		}

		void buildResponse();
		void buildParserErrorResponse();

		void sendResponse() {
			if (!responseReady()){
				return;
			}
			writeToSocket();
			if (writerFinished()) {
				clearParser();
				// client.setShouldDisconnect(true);// TODO KEEP-ALIVE/ keep connection?!
			}

		}

		EndpointResult processEvent(IoEvent ev) noexcept override {
			try {
				if (ev.isError) {
					return EndpointResult::ERROR;
				}
				if (ev.isRead) {
					receiveRequest();
				}
				if (ev.isWrite) {
					sendResponse();
				}
				if (getShouldDisconnect()) {
					return EndpointResult::ERROR;
				}
			} catch (std::exception &ex) {
				return EndpointResult::ERROR;
			}
			if (m_spawned.empty()) {
				return EndpointResult::EMPTY;
			}
			return EndpointResult::SPAWNED;
		}
};