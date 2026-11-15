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
				if (ev.isError) {
					m_cgiRunner->handleHangup();
				}
				if (m_cgiRunner->isDone()) {
					return EndpointResult::ERROR; // just to end things
				}
				if (ev.isWrite) { // write means we do pipe input
					if (m_type == PipeType::IN) {
						if (m_cgiRunner->doNextIn()) {
							return EndpointResult::ERROR;
						}
					} else {
						m_cgiRunner->forceKill();
						return EndpointResult::ERROR;
					}
				}
				if (ev.isRead) {
					if (m_type == PipeType::OUT) {
						m_cgiRunner->doNextOut();
					} else if (m_type == PipeType::ERR) {
						m_cgiRunner->doNextErr();
					} else {
						m_cgiRunner->forceKill();
						return EndpointResult::ERROR;
					}
				}
				if (m_cgiRunner->isDone()) {
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

		bool validate() override;
};
