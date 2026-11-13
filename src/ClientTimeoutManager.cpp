#include "ClientTimeoutManager.hpp"

ClientTimeoutManager::ClientTimeoutManager(int bodyT, int headerT, int kaT, int sendT, int cgiT, bool keepAlive) :
		m_clientBodyTimeout(bodyT), m_clientHeaderTimeout(headerT), m_keepAliveTimeout(kaT),
		m_sendTimeout(sendT), m_cgiTimeout(cgiT), m_keepAlive(keepAlive) {
	m_state = TimerState::WAITING_FOR_REQUEST;
	m_started = std::chrono::high_resolution_clock::now();
	m_lastRead = std::chrono::high_resolution_clock::now();
	m_lastWrite = std::chrono::high_resolution_clock::now();
}

void ClientTimeoutManager::startRequest() {
	m_state = TimerState::REQUEST_HEADERS;
	m_started = std::chrono::high_resolution_clock::now();
	m_lastRead = std::chrono::high_resolution_clock::now();
}

void ClientTimeoutManager::startBody() {
	m_state = TimerState::REQUEST_BODY;
	m_started = std::chrono::high_resolution_clock::now();
	m_lastRead = std::chrono::high_resolution_clock::now();
}

void ClientTimeoutManager::startCgi() {
	m_state = TimerState::CGI_WORKING;
	m_started = std::chrono::high_resolution_clock::now();
}

void ClientTimeoutManager::startResponse() {
	m_state = TimerState::RESPONSE;
	m_started = std::chrono::high_resolution_clock::now();
	m_lastWrite = std::chrono::high_resolution_clock::now();
}

void ClientTimeoutManager::finishResponse() {
	if (!m_keepAlive) {
		return;
	}
	m_state = TimerState::WAITING_FOR_REQUEST;
	m_started = std::chrono::high_resolution_clock::now();
}

void ClientTimeoutManager::readData() {
	m_lastRead = std::chrono::high_resolution_clock::now();
}

void ClientTimeoutManager::writtenData() {
	m_lastWrite = std::chrono::high_resolution_clock::now();
}

void ClientTimeoutManager::setKeepAlive(bool value) {
	m_keepAlive = value;
}

bool ClientTimeoutManager::isTimedOut()
{
	TimePoint now = std::chrono::high_resolution_clock::now();
	long sinceStarted = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_started).count();
	long sinceRead = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastRead).count();
	long sinceWritten = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastWrite).count();
	switch (m_state)
	{
	case TimerState::WAITING_FOR_REQUEST:
		if (!m_keepAlive) {
			return true;
		}
		return sinceStarted > m_keepAliveTimeout;
	case TimerState::REQUEST_HEADERS:
		if (sinceStarted > m_clientHeaderTimeout) {
			return true;
		}
	case TimerState::REQUEST_BODY:
		return sinceRead > m_clientBodyTimeout;
	case TimerState::CGI_WORKING:
		return sinceStarted > m_cgiTimeout; // the hangup and pipe errors are handled by the cgi processor itself
	case TimerState::RESPONSE:
		return sinceWritten > m_sendTimeout;
	}
	return true; // something is terribly wrong
}
