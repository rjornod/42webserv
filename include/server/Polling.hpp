#pragma once

#include "Client.hpp"
#include "IoEvent.hpp"
#include <vector>

class Poller {
	public:
		Poller() = default;
		Poller(Poller &) = delete;
		Poller &operator=(Poller &) = delete;
		Poller &operator=(Poller &&) = delete;

		virtual bool registerFd(int fd) = 0;
		virtual void unregisterFd(int fd) = 0;
		virtual std::vector<IoEvent> doPoll() = 0;
		virtual ~Poller() = default;
};

std::unique_ptr<Poller> createPoller();