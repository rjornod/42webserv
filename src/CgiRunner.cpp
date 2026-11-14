#include "CgiRunner.hpp"

#include <stdexcept>
#include <cstring>

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "HttpResponse.hpp"

static std::vector<char*> prepareEnv(RequestContext &ctx) {
	std::vector<char*> env;
	switch (ctx.getHttpRequest().getMethod())
	{
	case HttpMethod::GET:
		env.push_back(strdup("REQUEST_METHOD=GET"));
		break;
	case HttpMethod::POST:
		env.push_back(strdup("REQUEST_METHOD=POST"));
		break;
	case HttpMethod::DELETE:
		env.push_back(strdup("REQUEST_METHOD=DELETE"));
		break;
	default:
		// unreachable
		break;
	}
	auto headers = ctx.getHttpRequest().getHeaders();
	if (headers.find("content-type") != headers.end()) {
		env.push_back(strdup(("CONTENT_TYPE=" + headers.find("content-type")->second).c_str()));
		env.push_back(strdup(("CONTENT_LENGTH=" + std::to_string(ctx.getHttpRequest().getBodySize())).c_str()));
	}
	env.push_back(strdup("QUERY_STRING="));
	env.push_back(strdup("SERVER_PROTOCOL=HTTP/1.0"));
	env.push_back(strdup(("SCRIPT_NAME=" + ctx.getFilePath().string()).c_str()));
	env.push_back(strdup(("UPLOAD_DIR=" + ctx.getLocationConfig()->getUploadStore()).c_str()));
	env.push_back(nullptr);
	return env;
}

void CgiRunner::forceKill() {
	m_done = true;
	m_isError = true;
	if (waitpid(m_pid, nullptr, WNOHANG) == 0) {
		kill(m_pid, SIGQUIT);
	}
}

// need all the context
CgiRunner::CgiRunner(std::string path, RequestContext &ctx) : m_input(ctx.getHttpRequest().getBody()), m_written(0), m_done(false), m_isError(false) {
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
		std::vector<char*> env = prepareEnv(ctx);
		argv.push_back(strdup(path.c_str()));
		argv.push_back(strdup(ctx.getFilePath().c_str()));
		argv.push_back(nullptr);
		execve(path.c_str(), argv.data(), env.data());
		exit(1);
	}
	close(inFildes[0]);
	close(outFildes[1]);
	close(errFildes[1]);
}

CgiRunner::~CgiRunner() {
	if (waitpid(m_pid, nullptr, WNOHANG) == 0) {
		kill(m_pid, SIGQUIT);
	}
}

int CgiRunner::getInFd() const { return m_inFd; }
int CgiRunner::getOutFd() const { return m_outFd; }
int CgiRunner::getErrFd() const { return m_errFd; }

bool CgiRunner::doNextIn() {
	if (m_written >= m_input.length()) {
		return true;
	}
	ssize_t bytesWritten = write(m_inFd, m_input.c_str() + m_written, m_input.length() - m_written);
	if (bytesWritten <= 0) {
		forceKill();
		return true;
	}
	m_written += bytesWritten;
	return false;
}

void CgiRunner::doNextOut() {
	char buf[4096];
	ssize_t bytesRead = read(m_outFd, buf, 4096);
	if (bytesRead < 0) {
		forceKill();
		return;
	}
	if (bytesRead == 0) {
		m_done = true;
		return;
	}
	m_body += std::string(buf, bytesRead);
}

void CgiRunner::doNextErr() {
	char buf[4096];
	ssize_t bytesRead = read(m_errFd, buf, 4096);
	if (bytesRead < 0) {
		forceKill();
		return;
	}
	m_body = std::string(buf, bytesRead);
	m_isError = true;
	m_done = true;
}

void CgiRunner::handleHangup() {
	int exitStatus;
	if (waitpid(m_pid, &exitStatus, WNOHANG) == 0) {
		return;
	}
	if (!(WIFEXITED(exitStatus))) {
		return;
	}
	m_isError = (WEXITSTATUS(exitStatus)) != 0;
	m_done = true;
}

bool CgiRunner::isDone() const { return m_done; }

HttpResponse CgiRunner::getResponse() {
	if (!m_done) {
		throw std::runtime_error("requesting incomplete response");
	}
	HttpResponse response;
	if (m_isError) {
		response.setStatusCode(500);
	}
	response.makeStatusLine();
	response.addHeader("Content-Length", std::to_string(m_body.size()));
	response.addHeader("Content-Type", "text/plain");
	response.setBodySource(m_body);
	return response;
}
