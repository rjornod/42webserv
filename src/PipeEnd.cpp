#include "PipeEnd.hpp"

PipeEnd::PipeEnd(int fd, PipeType pipeType,
                 std::shared_ptr<CgiRunner> cgiRunner) : IoEndpoint(fd), m_type(pipeType), m_cgiRunner(cgiRunner) {}

bool PipeEnd::validate() {
	if (m_cgiRunner->isDone()) {
		return false;
	}
	return true;
}
