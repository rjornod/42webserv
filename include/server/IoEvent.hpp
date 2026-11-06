#pragma once

struct IoEvent {
	int fd;
	bool isRead;
	bool isWrite;
	bool isError;
};
