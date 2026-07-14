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

int ConfigParser::handleBraces(int i) {
	if (m_buffer[i] == '{') {
		m_tokens.emplace_back(Token{TokenType::OpenBrace, std::string(1, m_buffer[i])});
		++i;
	}
	else if (m_buffer[i] == '}') {
		m_tokens.emplace_back(Token{TokenType::ClosingBrace, std::string(1,m_buffer[i])});
		++i;
	}
	return i;
}

int ConfigParser::handleSemicolon(int i) {
	if (m_buffer[i] == ';') {
		m_tokens.emplace_back(Token{TokenType::Semicolon, std::string(1, m_buffer[i])});
		i++;
	}
	return i;
}

int ConfigParser::handleWord(int i) {
	int tokenStart = i;
	while (!isspace(m_buffer[i]) && m_buffer[i] != ';') {
		++i;
	}
	m_tokens.emplace_back(Token{TokenType::Word, m_buffer.substr(tokenStart, i - tokenStart)});
	handleSemicolon(i);
	i++;
	return i;
}

int ConfigParser::skipComments( int i) {
	if (m_buffer[i] == '#') {
		while (i < m_buffer.size() && m_buffer[i] != '\n')
			i++;
	}
	return i;
}

void ConfigParser::tokenize(std::fstream& file) {
	std::string line;
	while (std::getline(file, line)) {
		m_buffer += line;
		m_buffer += '\n';						// add a new line char to mark the end of a line
	}
	int i = 0;
	while (i< m_buffer.size()) {
		i = skipComments(i);
		if (isspace(m_buffer[i])) {
			i++;
			continue;
		}
		if (m_buffer[i] == '{' || m_buffer[i] == '}')
			i = handleBraces(i);
		if (!isspace(m_buffer[i]) && m_buffer[i] !=';')
			i = handleWord(i);
	}
	std::cout << BLUE << "----Printing tokens vector---" << std::endl;
	for (int i = 0; i < m_tokens.size(); i++) {
		std::cout << i << " - " <<  m_tokens[i] << std::endl;
	}
}