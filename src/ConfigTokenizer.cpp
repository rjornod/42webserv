#include "ConfigTokenizer.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

bool ConfigTokenizer::initialFileCheck(std::fstream& file) {
	
	if (!file.is_open()) {
		std::cout << RED << "Can't open config file. Please try again" << RESET << std::endl;
		return true;
	}
	bool isEmpty = file.peek() == EOF;
	if (isEmpty){
		std::cout << RED << "File is empty" << RESET << std::endl;
		return true;
	}
	return false;
}

bool ConfigTokenizer::parseFile() {
	std::fstream file(m_configPath);
	if (initialFileCheck(file))
		return true;
	tokenize(file);
	return false;
}

void ConfigTokenizer::tokenize(std::fstream& file) {
	std::string line;
	while (std::getline(file, line)) {
		
		tokens.push_back(line);
	}
	std::cout << BLUE << "----Printing token vector---" << RESET << std::endl;
	for (int i = 0; i < tokens.size(); i++) {
		std::cout << tokens[i] << std::endl;
	}
}