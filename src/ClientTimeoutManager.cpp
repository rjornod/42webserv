#include "ClientTimeoutManager.hpp"

ClientTimeoutManager::ClientTimeoutManager(bool keepAlive) : m_keepAlive(keepAlive) {
	m_state = TimerState::WAITING_FOR_REQUEST;
	m_started = std::chrono::high_resolution_clock::now();
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
