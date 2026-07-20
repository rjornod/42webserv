#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>  
#include "../include/ConfigParser.hpp"
#include "../include/ConfigParseExecption.hpp"
#include "../include/TokenType.hpp"
#include "../include/DirectiveType.hpp"
#include "ConfigParser.hpp"

int  tokenIndex = 0;

bool ConfigParser::initialFileCheck(std::fstream& file) {
	
	if (!file.is_open()) {
		std::cout << RED << "Can't open config file. Please try again" << RESET << std::endl;
		return true;
	}
	bool isEmpty = file.peek() == EOF;
	if (isEmpty){
		std::cout << RED << "File Error: " << RESET << " File is empty" << RESET << std::endl;
		return true;
	}
	return false;
}

bool ConfigParser::processConfig() {
	std::fstream file(m_configPath);
	if (initialFileCheck(file))
		return true;
	try {
		tokenize(file);
	}
	catch(const ConfigParseException& e) {
		std::cerr << RED << "File Error: " << RESET << e.what() << '\n';
		return true;
	}
	try {
		parseTokens();
	}
	catch (const ConfigParseException& e) {
		std::cerr << RED << "Config Error: " << RESET << e.what() << std::endl;
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

void ConfigParser::checkAllBraces() {
	int braceFound = 0;
	for (int i = 0; i < m_tokens.size(); i++) {
		if (m_tokens[i].type == TokenType::StartBlock)
			braceFound++;
		if (m_tokens[i].type == TokenType::EndBlock)
			braceFound--;
	}
	if (braceFound != 0)
		throw ConfigParseException("Detected unmatched brace");
	else
		std::cout << GREEN << "All braces are paired\n" << RESET <<std::endl;
}

void ConfigParser::parseTokens() {
	int  i = 0;
	int isGlobal = 0;
	checkAllBraces();
	// check if server block exists
	if (m_tokens[tokenIndex].value != "server" || m_tokens[tokenIndex + 1].type != TokenType::StartBlock) {
		throw ConfigParseException("Only server{} allowed as a global directive");
	}
	else {
		m_config.createServerConfig();
		parseBlock(true);
	}
}

void ConfigParser::parseBlock(bool isGlobal)
{
	std::cout << "parsing block. tokens values is:" << m_tokens[tokenIndex].value << "\n";
	if (isGlobal) {
		if (m_tokens[tokenIndex].value != "server")
			throw ConfigParseException("Only server{} allowed as a global directive");
	}
	else if (m_tokens[tokenIndex].value != "location") {
		throw ConfigParseException("Directive in server block is malformed " + m_tokens[tokenIndex].value);
	}
	std::cout << "block is location or server\n";
	tokenIndex++;
	tokenIndex++;
	while (m_tokens[tokenIndex].type != TokenType::EndBlock) {
		std::cout << "scanning directives\n";
		parseDirective();
		tokenIndex++;
	}
}

void ConfigParser::parseDirective()
{
	std::cout << "parsing directive. token is '" << m_tokens[tokenIndex].value <<"'\n"; 
	while (m_tokens[tokenIndex].type != TokenType::EndDirective) {
		if (m_tokens[tokenIndex].type != TokenType::Word || m_tokens[tokenIndex + 1].type != TokenType::Word)
			throw ConfigParseException("Directive is malformed");
		handleDirective();
		tokenIndex++;
		tokenIndex++;
	}
}

void ConfigParser::handleListen() {
	tokenIndex++;
	int port = std::stoi(m_tokens[tokenIndex].value);
	m_config.getServerConfigs().back().setPort(port);
	std::cout << m_config.getServerConfigs().back().getPort();
}

void ConfigParser::handleServerName() {
	tokenIndex++;
	m_config.getServerConfigs().back().setServerName(m_tokens[tokenIndex].value);
}

void ConfigParser::handleRoot() {
	tokenIndex++;
	m_config.getServerConfigs().back().setRoot(m_tokens[tokenIndex].value);
}

void ConfigParser::handleIndex() {
	tokenIndex++;
	m_config.getServerConfigs().back().setIndex(m_tokens[tokenIndex].value);
}

void ConfigParser::handleBodySize() {
	tokenIndex++;
	int bodySize = std::stoi(m_tokens[tokenIndex].value);
	m_config.getServerConfigs().back().setBodySize(bodySize);
}

void ConfigParser::handleDirective() {
	switch(directiveFromString(m_tokens[tokenIndex].value)) {
		case DirectiveType::Listen:
			handleListen();
			break;
		case DirectiveType::Name:
			handleServerName();
			break;
		case DirectiveType::Root:
			handleRoot();
			break;
		case DirectiveType::Index:
			handleIndex();
			break;
		case DirectiveType::ErrorPage:
			std::cout << "Directive: ErrorPage\n";
			break;
		case DirectiveType::Location:
			std::cout << "Directive: Location\n";
			break;
		case DirectiveType::AutoIndex:
			std::cout << "Directive: AutoIndex\n";
			break;
		case DirectiveType::MaxBodySize:
			std::cout << "Directive: MaxBodySize\n";
			break;
		case DirectiveType::Unknown:
			std::cout << "Unknown Directive\n";
			break;
	}

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
	if (m_tokens.empty())
		throw ConfigParseException("File doesn't have any server block");
	// printing to debug tokens
	// std::cout << BLUE << "----Printing tokens vector---" << RESET << std::endl;
	// for (int i = 0; i < m_tokens.size(); i++) {
	// 	std::cout << i << " - " <<  m_tokens[i] << std::endl;
	// }
	// 	std::cout << BLUE << "-----------------------------" << RESET << std::endl;
}