#pragma once

#include "IoEndpoint.hpp"
#include "CgiRunner.hpp"

class PipeEnd : public IoEndpoint {
	public:
		enum class PipeType {
			IN, OUT, ERR
		};
	private:
		PipeType m_type;
		std::shared_ptr<CgiRunner> m_cgiRunner;
	public:
		PipeEnd(int fd, PipeType pipeType, std::shared_ptr<CgiRunner> cgiRunner);

		EndpointResult processEvent(IoEvent ev) noexcept override {
			try {
				if (m_cgiRunner->isDone()) {
					return EndpointResult::ERROR; // just to end things
				}
				// TODO add hangup for pipes only
				if (ev.isError) {
					return EndpointResult::ERROR;
				}
				if (ev.isWrite) { // write means we do pipe input
					if (m_type == PipeType::IN) {
						m_cgiRunner->doNextIn();
					} else {
						// TODO kill cgi runner
						return EndpointResult::ERROR;
					}
				}
				if (ev.isRead) {
					if (m_type == PipeType::OUT) {
						m_cgiRunner->doNextOut();
					} else if (m_type == PipeType::ERR) {
						m_cgiRunner->doNextErr();
					} else {
						// TODO kill cgi runner
						return EndpointResult::ERROR;
					}
				}
				if (false) { // TODO check for validity
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
