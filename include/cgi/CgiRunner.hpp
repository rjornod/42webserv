#pragma once

#include <string>

class CgiRunner {
	public:
		CgiRunner() = default;
		CgiRunner(std::string path) {(void)path;};
};
