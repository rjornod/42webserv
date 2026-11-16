#pragma once

#include "RequestContext.hpp"
#include "ClientTimeoutManager.hpp"

class HttpResponse;

class CgiRunner {
	private:
		pid_t m_pid;
		int m_inFd;
		int m_outFd;
		int m_errFd;

		const std::string m_input;
		size_t m_written;

		bool m_done;
		bool m_isError;
		std::string m_body;
		std::string m_setCookie;

		ClientTimeoutManager *m_toManager;
	public:
		CgiRunner() = delete;
		CgiRunner(std::string path, RequestContext &ctx);
		~CgiRunner();
		
		int getInFd() const;
		int getOutFd() const;
		int getErrFd() const;
		
		void forceKill();
		bool doNextIn();
		void doNextOut();
		void doNextErr();
		void handleHangup();

		void setTimeoutManager(ClientTimeoutManager *toManager);

		void checkTimeout();
		bool isDone() const;
		HttpResponse getResponse();
};
