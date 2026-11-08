#include "CgiRunner.hpp"

#include <stdexcept>
#include <cstring>

#include <unistd.h>

#include "HttpResponse.hpp"

// need all the context
CgiRunner::CgiRunner(std::string path, RequestContext &ctx) : m_done(false) {
	// TODO prepare env
	int	inFildes[2];
	int	outFildes[2];
	int	errFildes[2];

	if (pipe(inFildes) != 0) {
		perror("in pipe");
		throw std::runtime_error("in pipe");
	}
	if (pipe(outFildes) != 0) {
		close(inFildes[0]);
		close(inFildes[1]);
		perror("out pipe");
		throw std::runtime_error("out pipe");
	}
	if (pipe(errFildes) != 0) {
		close(inFildes[0]);
		close(inFildes[1]);
		close(outFildes[0]);
		close(outFildes[1]);
		perror("err pipe");
		throw std::runtime_error("err pipe");
	}

	m_inFd = inFildes[1];
	m_outFd = outFildes[0];
	m_errFd = errFildes[0];

	m_pid = fork();
	if (m_pid < 0) {
		close(inFildes[0]);
		close(inFildes[1]);
		close(outFildes[0]);
		close(outFildes[1]);
		close(errFildes[0]);
		close(errFildes[1]);
		throw std::runtime_error("fork error");
	}
	if (m_pid == 0) {
		close(m_inFd);
		close(m_outFd);
		close(m_errFd);
		// TODO change workdir
		if (dup2(inFildes[0], STDIN_FILENO) < 0) {
			perror("dup2");
			exit(1);
		}
		if (dup2(outFildes[1], STDOUT_FILENO) < 0) {
			perror("dup2");
			exit(1);
		}
		if (dup2(errFildes[1], STDERR_FILENO) < 0) {
			perror("dup2");
			exit(1);
		}
		std::vector<char*> argv;
		std::vector<char*> env(1);
		argv.push_back(strdup(path.c_str()));
		argv.push_back(strdup(ctx.getFilePath().c_str()));
		argv.push_back(nullptr);
		execve(path.c_str(), argv.data(), env.data());
		free(argv[0]);
		free(argv[1]);
		exit(1);
	}
	close(inFildes[0]);
	close(outFildes[1]);
	close(errFildes[1]);
}

int CgiRunner::getInFd() const { return m_inFd; }
int CgiRunner::getOutFd() const { return m_outFd; }
int CgiRunner::getErrFd() const { return m_errFd; }

void CgiRunner::doNextIn() {
	// TODO write body into pipe in
}

void CgiRunner::doNextOut() {
	char buf[4096];
	ssize_t bytesRead = read(m_outFd, buf, 4096);
	if (bytesRead < 0) {
		// TODO process error
	}
	m_body = std::string(buf, bytesRead);
	m_done = true;
}

void CgiRunner::doNextErr() {
	// TODO
}

bool CgiRunner::isDone() const { return m_done; }

HttpResponse CgiRunner::getResponse() {
	HttpResponse response;
	response.makeStatusLine();
	response.addHeader("Content-Length", std::to_string(m_body.size()));
	response.addHeader("Content-Type", "text/plain");
	response.setBodySource(m_body);
	return response;
}
