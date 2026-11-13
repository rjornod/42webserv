#pragma once

#include <chrono>

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

class ClientTimeoutManager {
	private:
		int m_clientBodyTimeout; // between 2 successive reads
		int m_clientHeaderTimeout;
		int m_keepAliveTimeout;
		int m_sendTimeout; // between 2 successive writes
		int m_cgiTimeout;

		TimePoint m_started;
		TimePoint m_lastRead;
		TimePoint m_lastWrite;

		bool m_keepAlive;
		enum class TimerState {
			WAITING_FOR_REQUEST, REQUEST_HEADERS, REQUEST_BODY, CGI_WORKING, RESPONSE
		} m_state;
	public:
		ClientTimeoutManager(int bodyT, int headerT, int kaT, int sendT, int cgiT, bool keepAlive = false);
		void startRequest();
		void startBody();
		void startCgi();
		void startResponse();
		void finishResponse();
		void readData();
		void writtenData();
		void setKeepAlive(bool value);

		bool isTimedOut();
};
