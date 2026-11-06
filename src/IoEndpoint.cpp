#include "IoEndpoint.hpp"

#include <unistd.h>
#include <utility>

IoEndpoint::IoEndpoint(int fd) : m_fd(fd) {}

void IoEndpoint::spawn(IoEndpointBox &&ep) {
	m_spawned.push_back(std::move(ep));
}

IoEndpoint::~IoEndpoint() {
	close(m_fd);
}

int IoEndpoint::getFd() const noexcept {
	return m_fd;
}

IoEndpointList IoEndpoint::collectSpawned() noexcept {
  return std::exchange(m_spawned, {});
}
