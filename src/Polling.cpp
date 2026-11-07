#include "Polling.hpp"

#include <poll.h>

class DefaultPoller : public Poller {
	private:
		std::vector<pollfd> m_connectedFds;

	public:
		DefaultPoller() : Poller() {};

		virtual bool registerFd(int fd) override {
			m_connectedFds.emplace_back(pollfd{fd, POLLIN | POLLOUT | POLLRDHUP, 0});
			return true;
		};

		virtual void unregisterFd(int fd) override {
			auto it = m_connectedFds.begin();
			while (it != m_connectedFds.end() && it->fd != fd) {
				it = std::next(it);
			}
			if (it != m_connectedFds.end()) {
				m_connectedFds.erase(it);
			}
		};

		virtual std::vector<IoEvent> doPoll() override {
			int returnValue =
				poll(m_connectedFds.data(), m_connectedFds.size(), 0); // 0 timeout makes poll constantly check all the clients
			if (returnValue < 0) {
				if (errno == EINTR) {
					return {};
				}
				perror("poll");
				throw std::exception();
			}
			std::vector<IoEvent> result;
			for (const auto& entry : m_connectedFds) {
				if (!entry.revents) {
					continue;
				}
				IoEvent ev{entry.fd, false, false, false};
				if (entry.revents & POLLIN) {
					ev.isRead = true;
				}
				if (entry.revents & POLLOUT) {
					ev.isWrite = true;
				}
				if (entry.revents & POLLERR) {
					ev.isError = true;
				}
				if (entry.revents & POLLHUP) {
					ev.isError = true;
				}
				if (entry.revents & POLLNVAL) {
					ev.isError = true;
				}
				if (entry.revents & POLLRDHUP) {
					ev.isError = true;
				}
				result.push_back(ev);
			}
			return result;
		};

		virtual ~DefaultPoller() override {};
};

std::unique_ptr<Poller> createPoller() {
	return std::make_unique<DefaultPoller>();
}