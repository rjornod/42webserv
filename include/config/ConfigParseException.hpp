#pragma once
#include <stdexcept>
#include <iostream>

class ConfigParseException : public std::runtime_error {
	public:
		explicit ConfigParseException(const std::string& error) 
			: std::runtime_error(error) {}
};