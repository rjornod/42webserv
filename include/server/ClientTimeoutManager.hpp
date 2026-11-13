#pragma once

#include <chrono>

using timePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

class ClientTimeoutManager {
	private:
		int m_clientBodyTimeout; // between 2 successive reads
		int m_clientHeaderTimeout;
		int m_keepAliveTimeout;
		int m_sendTimeout; // between 2 successive writes
		int m_cgiTimeout; // TODO maybe move to special class

		timePoint m_started;
		timePoint m_lastRead;
		timePoint m_lastWrite;

		bool m_keepAlive;
		enum class TimerState {
			WAITING_FOR_REQUEST, REQUEST_HEADERS, REQUEST_BODY, RESPONSE
		} m_state;
	public:
		ClientTimeoutManager(bool keepAlive = false);
		void startRequest();
		void startBody();
		void startResponse();
		void finishResponse();
		void readData();
		void writtenData();
		void setKeepAlive(bool value);
};
