#include <fstream>
#include <filesystem>
#include <iostream>
#include "../include/ConfigParser.hpp"
#include "ConfigParser.hpp"
#include "../include/TokenType.hpp"


bool ConfigParser::initialFileCheck(std::fstream& file) {
	
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

bool ConfigParser::parseFile() {
	std::fstream file(m_configPath);
	if (initialFileCheck(file))
		return true;
	tokenize(file);
	return false;
}

void ConfigParser::tokenize(std::fstream& file) {
	std::string line;
	// std::string buffer;
	while (std::getline(file, line)) {
		m_buffer += line;
	}
	int i = 0;
	while (i< m_buffer.size()) {
		if (isspace(m_buffer[i])) {
			i++;
			continue;
		}
		if (m_buffer[i] == '{') {
			m_tokens.emplace_back(Token{TokenType::OpenBrace, std::string(1, m_buffer[i])});
			// std::cout << "Token position: " << i <<" - "  << i << ": '" << m_buffer[i] << "'" << std::endl;
			++i;
		}
		else if (m_buffer[i] == '}') {
			m_tokens.emplace_back(Token{TokenType::ClosingBrace, std::string(1,m_buffer[i])});
			++i;
		}
		else {
			int tokenStart = i;
			while (!isspace(m_buffer[i]) && m_buffer[i] != ';') {
				++i;
			}
			m_tokens.emplace_back(Token{TokenType::Word, m_buffer.substr(tokenStart, i - tokenStart)});
			// std::cout << "Token position: " << tokenStart <<" - "  << i << ": '" << m_buffer.substr(tokenStart, i - tokenStart) << "'" << std::endl;
			if (m_buffer[i] == ';') {
				m_tokens.emplace_back(Token{TokenType::Semicolon, std::string(1, m_buffer[i])});
				// std::cout << "Token position: " << i <<" - "  << i << ": '" << m_buffer[i] << "'" << std::endl;
				i++;
			}
		}

	}
	std::cout << BLUE << "----Printing tokens vector---" << std::endl;
	for (int i = 0; i < m_tokens.size(); i++) {
		std::cout << i << " - " <<  m_tokens[i] << std::endl;
	}
}