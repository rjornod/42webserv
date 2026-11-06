#pragma once

#include <memory>
#include <vector>

#include "IoEvent.hpp"

class IoEndpoint;

using IoEndpointBox = std::unique_ptr<IoEndpoint>;
using IoEndpointList = std::vector<IoEndpointBox>;

class IoEndpoint {
	protected:
	int m_fd;
		IoEndpointList m_spawned;

		IoEndpoint(int fd);
		void spawn(IoEndpointBox &&ep);

	public:
		enum class EndpointResult {
			EMPTY,
			SPAWNED,
			ERROR,
		};

		IoEndpoint(IoEndpoint &) = delete;
		IoEndpoint &operator=(IoEndpoint &) = delete;
		IoEndpoint &operator=(IoEndpoint &&) = delete;
		virtual ~IoEndpoint();

		int getFd() const noexcept;
		IoEndpointList collectSpawned() noexcept;

		virtual EndpointResult processEvent(IoEvent ev) noexcept = 0;
};
