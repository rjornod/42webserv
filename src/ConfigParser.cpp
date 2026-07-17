#include <fstream>
#include <filesystem>
#include <iostream>
#include "../include/ConfigParser.hpp"
#include "ConfigParser.hpp"
#include "../include/ConfigParseExecption.hpp"
#include "../include/TokenType.hpp"

int  tokenIndex = 0;

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
	try {
		parseTokens();
	} catch (const ConfigParseException& e) {
		std::cerr << RED << "Config error: " << RESET << e.what() << std::endl;
		return true;
	}
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
		std::cout << GREEN << "All braces are paired" << RESET << std::endl;
	if (m_tokens[tokenIndex].value != "server" || m_tokens[tokenIndex + 1].type != TokenType::StartBlock) {
		throw ConfigParseException("only server{} allowed as a global directive");
		return true;
	}
	// if (m_tokens[tokenIndex].type != TokenType::StartBlock) {
	// 	std::cout << RED << "Block not correctly initialized" << RESET << std::endl;
	// 	return true;
	// }
	tokenIndex++;
	if (parseBlock(true)) 
		return true;
	return false;
}

bool ConfigParser::parseBlock(bool isGlobal)
{
	if (isGlobal) {
		if (m_tokens[tokenIndex].value != "server")
			return true;
	else if (m_tokens[tokenIndex].value != "location")
		return true;
	}
	tokenIndex++;
	while (m_tokens[tokenIndex].type != TokenType::EndBlock) {
		if (parseDirective())
			return true;
		tokenIndex++;
	}
	return false;
}

bool ConfigParser::parseDirective()
{
	while (m_tokens[tokenIndex].type != TokenType::EndDirective) {
		if (m_tokens[tokenIndex].type != TokenType::Word)
			return true;
	}
	
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
	// std::cout << BLUE << "----Printing tokens vector---" << RESET << std::endl;
	// for (int i = 0; i < m_tokens.size(); i++) {
	// 	std::cout << i << " - " <<  m_tokens[i] << std::endl;
	// }
	// 	std::cout << BLUE << "-----------------------------" << RESET << std::endl;
}