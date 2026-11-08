#pragma once

#include "RequestContext.hpp"

class HttpResponse;

class CgiRunner {
	private:
		pid_t m_pid;
		int m_inFd;
		int m_outFd;
		int m_errFd;

		bool m_done;
		std::string m_body;
	public:
		CgiRunner() = delete;
		CgiRunner(std::string path, RequestContext &ctx);

		int getInFd() const;
		int getOutFd() const;
		int getErrFd() const;

		void doNextIn();
		void doNextOut();
		void doNextErr();

		bool isDone() const;
		HttpResponse getResponse();
};
