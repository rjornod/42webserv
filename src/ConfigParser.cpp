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

bool ConfigParser::processConfig() {
	std::fstream file(m_configPath);
	if (initialFileCheck(file))
		return true;
	tokenize(file);
	if (parseTokens())
		return true;
	return false;
}

int ConfigParser::handleBraces(int i) {
	if (m_buffer[i] == '{') {
		m_tokens.emplace_back(Token{TokenType::StartBlock, std::string(1, m_buffer[i])});
		++i;
	}
	else if (m_buffer[i] == '}') {
		m_tokens.emplace_back(Token{TokenType::EndBlock, std::string(1,m_buffer[i])});
		++i;
	}
	return i;
}

int ConfigParser::handleEndDirective(int i) {
	if (m_buffer[i] == ';') {
		m_tokens.emplace_back(Token{TokenType::EndDirective, std::string(1, m_buffer[i])});
		i++;
	}
	return i;
}

int ConfigParser::handleWord(int i) {
	int tokenStart = i;
	while (!isspace(m_buffer[i]) && m_buffer[i] != ';'  && m_buffer[i] !='{' && m_buffer[i] !='}') {
		++i;
	}
	m_tokens.emplace_back(Token{TokenType::Word, m_buffer.substr(tokenStart, i - tokenStart)});
	return i;
}

int ConfigParser::skipComments(int i) {
	if (m_buffer[i] == '#') {
		while (i < m_buffer.size())
			i++;
	}
	return i;
}

bool ConfigParser::checkAllBraces() {
	int braceFound = 0;
	for (int i = 0; i < m_tokens.size(); i++) {
		if (m_tokens[i].type == TokenType::StartBlock)
			braceFound++;
		if (m_tokens[i].type == TokenType::EndBlock)
			braceFound--;
	}
	if (braceFound != 0)
		return true;
	return false;
}

bool ConfigParser::parseTokens() {
	int  i = 0;
	int isGlobal = 0;
	if (checkAllBraces()) {
		std::cout << RED << "Not all braces are closed" << RESET << std::endl;
		return true;
	}
	else
		std::cout << "All braces are paired" << std::endl;
	if (m_tokens[i].type != TokenType::Word || m_tokens[i].value != "server") {
		std::cout << RED << "ServerBlock not Found" << RESET << std::endl;
		return true;
	}
	i++;
	if (m_tokens[i].type != TokenType::StartBlock || m_tokens[i].value != "{") {
		std::cout << RED << "Block not correctly initialized" << RESET << std::endl;
		return true;
	}
	i++;
	parseBlock(i, 1);
	return false;
}

bool ConfigParser::parseBlock(int i, bool isGlobal)
{
	if (isGlobal) {
		if (m_tokens[i].value != "server")
			return true;
	else if (m_tokens[i].value != "location")
				return true;
	}
	i++;
	while (m_tokens[i].type != TokenType::EndBlock) {
		if (parseDirective(i))
			return true;
		i++;
	}
	return false;
}

bool ConfigParser::parseDirective(int i)
{
	if (m_tokens[i].type != TokenType::Word)
		return true;
	
	return false;
}

void ConfigParser::tokenize(std::fstream& file) {
	while (std::getline(file, m_buffer)) {
		if (m_buffer.empty()) {																			// skip empty lines
			continue;
		}
		int i = 0;
		while (i < m_buffer.size()) {
			if (m_buffer[i] == '#' && i < m_buffer.size()) {
				i = skipComments(i);
				continue;
			}
			if (isspace(m_buffer[i]) && i < m_buffer.size()) {
				i++;
				continue;
			}
			if (m_buffer[i] == ';' && i < m_buffer.size()) {
				i = handleEndDirective(i);
				continue;
			}
			if (m_buffer[i] == '{' || m_buffer[i] == '}' && i < m_buffer.size()) {
				i = handleBraces(i);
				continue;
			}
			if (!isspace(m_buffer[i]) && m_buffer[i] !=';' && i < m_buffer.size()) {
				i = handleWord(i);
				continue;
			}
		}
	}
	// printing to debug tokens
	std::cout << BLUE << "----Printing tokens vector---" << RESET << std::endl;
	for (int i = 0; i < m_tokens.size(); i++) {
		std::cout << i << " - " <<  m_tokens[i] << std::endl;
	}
		std::cout << BLUE << "-----------------------------" << RESET << std::endl;
}