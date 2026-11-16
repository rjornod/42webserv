#include "Polling.hpp"

#include <poll.h>
#include <sys/epoll.h>
#include <cstring>

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

class EpollPoller : public Poller {
	private:
		const int EVENT_MASK = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP;
  		int m_EpollFd;

	public:
		EpollPoller() : Poller() {
			m_EpollFd = epoll_create(1);
			if (m_EpollFd < 0) {
			throw std::exception();
			}
		};

		virtual bool registerFd(int fd) override {
			epoll_event ev;
			std::memset(&ev, 0, sizeof(ev));
			ev.events = EVENT_MASK;
			ev.data.fd = fd;
			return epoll_ctl(m_EpollFd, EPOLL_CTL_ADD, fd, &ev) == 0;
		};

		virtual void unregisterFd(int fd) override {
			epoll_ctl(m_EpollFd, EPOLL_CTL_DEL, fd, nullptr);
		};

		virtual std::vector<IoEvent> doPoll() override {
			const int MAX_EVENTS = 128;
			epoll_event events[MAX_EVENTS];
			int numEvents = epoll_wait(m_EpollFd, events, MAX_EVENTS, 0);
			if (numEvents < 0) {
			throw std::exception();
			}
			std::vector<IoEvent> result;
			for (int i = 0; i < numEvents; i++) {
				IoEvent ev{events[i].data.fd, false, false, false};
				if (events[i].events & EPOLLIN) {
					ev.isRead = true;
				}
				if (events[i].events & EPOLLOUT) {
					ev.isWrite = true;
				}
				if (events[i].events & EPOLLRDHUP) {
					ev.isError = true;
				}
				if (events[i].events & EPOLLPRI) {
					ev.isError = true;
				}
				if (events[i].events & EPOLLERR) {
					ev.isError = true;
				}
				if (events[i].events & EPOLLHUP) {
					ev.isError = true;
				}
				result.push_back(ev);
			}
			return result;
		};

		virtual ~EpollPoller() override {
			close(m_EpollFd);
		};
};

std::unique_ptr<Poller> createPoller() {
	return std::make_unique<EpollPoller>();
}