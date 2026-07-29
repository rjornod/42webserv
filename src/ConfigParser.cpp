#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>  
#include "../include/ConfigParser.hpp"
#include "../include/ConfigParseExecption.hpp"
#include "../include/TokenType.hpp"
#include "../include/DirectiveType.hpp"
#include "../include/LocationDirectiveType.hpp"
#include "ConfigParser.hpp"

#define GLOBAL 1
#define LOCATION 0

unsigned int  tokenIndex = 0;

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

int ConfigParser::skipComments(unsigned long i) {
	if (m_buffer[i] == '#') {
		while (i < m_buffer.size())
			i++;
	}
	return i;
}

void ConfigParser::checkAllBraces() {
	int braceFound = 0;
	for (unsigned long i = 0; i < m_tokens.size(); i++) {
		if (isType(m_tokens[i], TokenType::StartBlock))
			braceFound++;
		if (isType(m_tokens[i], TokenType::EndBlock))
			braceFound--;
	}
	if (braceFound != 0)
		throw ConfigParseException("Detected unmatched brace");
	else
		std::cout << GREEN << "All braces are paired\n" << RESET <<std::endl;
}
void ConfigParser::parseTokens() {
	checkAllBraces();
	// check if server block exists
	if (tokenIndex + 1 >= m_tokens.size() || !isValue(m_tokens[tokenIndex], "server")
		|| !isType(m_tokens[tokenIndex + 1], TokenType::StartBlock)) {
		throw ConfigParseException("Only server{} allowed as a global directive");
	}
	else {
		m_config.createServerConfig();
		parseBlock(true);
	}
}

void ConfigParser::parseBlock(bool isGlobal)
{
	if (isGlobal) {
		if (!isValue(m_tokens[tokenIndex], "server"))
			throw ConfigParseException("Only server{} allowed as a global directive");
	}
	else if (!isValue(m_tokens[tokenIndex], "location")) {
		throw ConfigParseException("Directive in server block is malformed " + m_tokens[tokenIndex].value);
	}	
	checkIfBlockEmpty("server");			
	incTokenIndex(2);																										// skip server or location token and StartBlock token
	while (!isType(m_tokens[tokenIndex], TokenType::EndBlock)) {
		parseDirective();
		incTokenIndex(1);		
	}
}

void ConfigParser::parseDirective()
{
	if (isValue(m_tokens[tokenIndex], "location")) {
		if (tokenIndex + 2 >= m_tokens.size() || !isType(m_tokens[tokenIndex + 1], TokenType::Word)
			|| !isType(m_tokens[tokenIndex + 2], TokenType::StartBlock))
			throw ConfigParseException("Location directivddddde is malformed");
		checkIfBlockEmpty("location");
		m_config.getServerConfigs().back().createLocationConfig();	
		parseLocationDirectives();
	}
	else
		handleDirective();	
}

void ConfigParser::parseLocationDirectives() {
	incTokenIndex(1); 																												// skip location token
	m_config.getServerConfigs().back().getLocationConfigs().back().setPath(m_tokens[tokenIndex].value);
	incTokenIndex(2);
	while (!isType(m_tokens[tokenIndex], TokenType::EndDirective)) {
		if (isType(m_tokens[tokenIndex], TokenType::EndBlock))
			return;
		handleLocationDirective();
		incTokenIndex(1);
	}
		incTokenIndex(1);
}

void ConfigParser::checkEndOfDirective(std::string directive) {
	incTokenIndex(1);
	if 	(isValidToken(m_tokens[tokenIndex]))
	 	throw ConfigParseException(directive + " directive has too many arguments");
	if (!isType(m_tokens[tokenIndex], TokenType::EndDirective))
		throw ConfigParseException(directive + " directive is missing a semicolon");
}

static bool validateDigits(std::string port) {
	for (unsigned long i = 0; i < port.size(); i++) {
		if (!isdigit(port[i]))
			return false;
	}
	return true;
}

void ConfigParser::handleListen() {
	incTokenIndex(1);
	if 	(!isType(m_tokens[tokenIndex], TokenType::Word) || 
			!isValidToken(m_tokens[tokenIndex]))
		throw ConfigParseException("listen directive is missing argument");
	if (m_tokens[tokenIndex].value.size() > 5)
		throw ConfigParseException("Port '" + m_tokens[tokenIndex].value + "' is not valid. Must be an integer with max value of 65535");
	if (!validateDigits(m_tokens[tokenIndex].value))
		throw ConfigParseException("Port argument is not a valid integer");
	int port = std::stoi(m_tokens[tokenIndex].value);
	if (port < 1024 || port > 65535)
		throw ConfigParseException("Port has to be a value between 1024 and 65535");
	m_config.getServerConfigs().back().setPort(port);
	checkEndOfDirective("listen");
	std::cout << GREEN << "LISTEN OK\n" << RESET;
}

void ConfigParser::handleServerName() {
	incTokenIndex(1);
	if 	(!isType(m_tokens[tokenIndex], TokenType::Word) || 
			!isValidToken(m_tokens[tokenIndex]))
		throw ConfigParseException("server_name directive is missing argument");
	m_config.getServerConfigs().back().setServerName(m_tokens[tokenIndex].value);
	checkEndOfDirective("server_name");
	std::cout << GREEN << "NAME OK\n" << RESET;
}

void ConfigParser::handleRoot(int scope) {
	incTokenIndex(1);
	if 	(!isType(m_tokens[tokenIndex], TokenType::Word) || 
			!isValidToken(m_tokens[tokenIndex]))
		throw ConfigParseException("root directive is missing argument");
	if (scope == GLOBAL)
		m_config.getServerConfigs().back().setRoot(m_tokens[tokenIndex].value);
	else 
		m_config.getServerConfigs().back().getLocationConfigs().back().setRoot(m_tokens[tokenIndex].value);
	checkEndOfDirective("root");
	std::cout << GREEN << "ROOT OK\n" << RESET;
}

void ConfigParser::handleIndex(int scope) {
	incTokenIndex(1);
	if (!isType(m_tokens[tokenIndex], TokenType::Word))
		throw ConfigParseException("Index directive is malformed");
	// check if the current token is a Word and is not the same as a known directive
	while (tokenIndex < m_tokens.size() && 
				isValidToken(m_tokens[tokenIndex])) {
		if (scope == GLOBAL)
			m_config.getServerConfigs().back().setIndex(m_tokens[tokenIndex].value);
		else
			m_config.getServerConfigs().back().getLocationConfigs().back().setIndex(m_tokens[tokenIndex].value);
		incTokenIndex(1);
	}
	if (!isType(m_tokens[tokenIndex], TokenType::EndDirective)) 
		throw ConfigParseException("Index directive is missing a semicolon");
	std::cout << GREEN << "INDEX OK\n" << RESET;
}

void ConfigParser::handleBodySize(int scope) {
	incTokenIndex(1);
	if (!validateDigits(m_tokens[tokenIndex].value))
		throw ConfigParseException("client_max_body_size argument is not a valid integer");
	int bodySize = std::stoi(m_tokens[tokenIndex].value);
	if (scope == GLOBAL)
		m_config.getServerConfigs().back().setBodySize(bodySize);
	else
		m_config.getServerConfigs().back().getLocationConfigs().back().setBodySize(bodySize);
	checkEndOfDirective("client_max_body_size");
	std::cout << GREEN << "CLIENTMAXBODYSIZE OK\n" << RESET;
}

void ConfigParser::handleUnknown() {
	std::cout << m_tokens[tokenIndex] << "\n";
	throw ConfigParseException("Unknown directive detected");
}

void ConfigParser::handleAutoIndex(int scope) {
	incTokenIndex(1);
	if 	(!isType(m_tokens[tokenIndex], TokenType::Word) || 
			!isValidToken(m_tokens[tokenIndex]))
		throw ConfigParseException("autoindex directive is missing argument");
	if (m_tokens[tokenIndex].value != "on" && m_tokens[tokenIndex].value != "off")
		throw ConfigParseException("autoindex argument is not correct. Only 'on' or 'off' allowed");
	// TO DO: find a cleaner way to do the next assigment 
	if (m_tokens[tokenIndex].value == "on" && scope == GLOBAL)
		m_config.getServerConfigs().back().setAutoIndex(true);
	else if (m_tokens[tokenIndex].value == "off" && scope == GLOBAL)
		m_config.getServerConfigs().back().setAutoIndex(false);
	else if (m_tokens[tokenIndex].value == "on" && scope == LOCATION)
		m_config.getServerConfigs().back().getLocationConfigs().back().setAutoIndex(true);
	else if (m_tokens[tokenIndex].value == "off" && scope == LOCATION)
		m_config.getServerConfigs().back().getLocationConfigs().back().setAutoIndex(false);

	checkEndOfDirective("autoindex");
	std::cout << GREEN << "AUTOINDEX OK\n" << RESET;

}

int ConfigParser::validateErrorCode(std::string errorCode) {
	
	if (errorCode.size() != 3)
		throw ConfigParseException("Error code in error_pages is invalid: " + errorCode );
	for (unsigned long i = 0; i < errorCode.size(); i++) {
		if (!isdigit(errorCode[i]))
			throw ConfigParseException("Error code must contain only digits: " + errorCode);
	}
	int code = std::stoi(errorCode);
	if (code < 100 || code > 500)
		throw ConfigParseException("Error code must be between 100 and 500: " + errorCode);
	return code;
}

std::string ConfigParser::checkURI() {
	unsigned long i = tokenIndex;
	while (!isType(m_tokens[i + 1], TokenType::EndDirective) || isValidToken(m_tokens[i + 1]))	{
		i++;

	}
	if (m_tokens[i].value[0] != '/')
		throw ConfigParseException("Error path should be absolute. Example: '/error.html' or '/errors/404.html'");
	return m_tokens[i].value;
}

void ConfigParser::handleErrorPages(int scope) {
	incTokenIndex(1);
	std::string path = checkURI();
	// unsigned int startToken = tokenIndex;
	if (!isType(m_tokens[tokenIndex], TokenType::Word) || !isType(m_tokens[tokenIndex + 1], TokenType::Word))
		throw ConfigParseException("error_pages directive is malformed");
	while (tokenIndex < m_tokens.size() && 
				isValidToken(m_tokens[tokenIndex]) &&
				!isType(m_tokens[tokenIndex + 1], TokenType::EndDirective) ) {
		if (scope == GLOBAL)
			m_config.getServerConfigs().back().setErrorPages(validateErrorCode(m_tokens[tokenIndex].value), path);
		else if (scope == LOCATION)
			m_config.getServerConfigs().back().getLocationConfigs().back().setErrorPages(validateErrorCode(m_tokens[tokenIndex].value), path);
		incTokenIndex(1);
	}
	incTokenIndex(1); // TO DO: handle path
	if (!isType(m_tokens[tokenIndex], TokenType::EndDirective)) 
		throw ConfigParseException("error_pages directive is missing a semicolon");
	std::cout << GREEN << "ERRORPAGES OK\n" << RESET;
}

void ConfigParser::handleLocation() {
	std::cout <<"handling location\n";
	incTokenIndex(1);
	parseBlock(false);
	std::cout << GREEN << "LOCATION OK\n" << RESET;
}

void ConfigParser::handleLocationDirective() {
	switch(locationDirectiveFromString(m_tokens[tokenIndex].value)) {
		case LocationDirectiveType::Root:
			handleRoot(LOCATION);
			break;
		case LocationDirectiveType::AutoIndex:
			handleAutoIndex(LOCATION);
			std::cout << "Directive: location autoindex\n";
			break;
		case LocationDirectiveType::MaxBodySize:
			handleBodySize(LOCATION);
			break;
		case LocationDirectiveType::Index:
			handleIndex(LOCATION);
			break;
		case LocationDirectiveType::ErrorPage:
			handleErrorPages(LOCATION);
			std::cout << "Directive: ErrorPage\n";
			break;
		case LocationDirectiveType::UploadStore:
			incTokenIndex(1);
			std::cout << "Directive: UploadStore\n";
			break;
		case LocationDirectiveType::LimitExcept:
			incTokenIndex(1);
			std::cout << "Directive: LimitExcept\n";
			break;
		case LocationDirectiveType::Return:
			incTokenIndex(1);
			std::cout << "Directive: Return\n";
			break;
		case LocationDirectiveType::Unknown:
		std::cout << "Unknown Drrrrirective " << m_tokens[tokenIndex] << "\n";
			// handleUnknown();
			break;
	}
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
			handleRoot(GLOBAL);
			break;
		case DirectiveType::Index:  
			handleIndex(GLOBAL);
			break;
		case DirectiveType::ErrorPage:
			handleErrorPages(GLOBAL);
			break;
		case DirectiveType::Location:
			handleLocation();
			break;
		case DirectiveType::AutoIndex:
			std::cout << "handling auto index\n";
			handleAutoIndex(GLOBAL);
			break;
		case DirectiveType::MaxBodySize:
			handleBodySize(GLOBAL);
			break;
		case DirectiveType::Unknown:
			std::cout << "Unknown Directive\n";
			handleUnknown();
			break;
	}
}

void ConfigParser::tokenize(std::fstream& file) {
	while (std::getline(file, m_buffer)) {
		if (m_buffer.empty()) {																			// skip empty lines
			continue;
		}
		unsigned long i = 0;
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
			if ((m_buffer[i] == '{' || m_buffer[i] == '}') && (i < m_buffer.size())) {
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
}

void ConfigParser::printTokens() {
	std::cout << BLUE << "----Printing tokens vector---" << RESET << std::endl;
	for (unsigned long i = 0; i < m_tokens.size(); i++) {
		std::cout << i << " - " <<  m_tokens[i] << std::endl;
	}
		std::cout << BLUE << "-----------------------------" << RESET << std::endl;
}

bool ConfigParser::isValidToken(const Token& token) {
	return 	isType(token, TokenType::Word) && 
					!knownDirectives.count(token.value);
}

bool	ConfigParser::isType(const Token& token, TokenType expectedType) {
	return token.type == expectedType;
}

bool	ConfigParser::isValue(const Token& token, const std::string& expectedValue) {
	return token.value == expectedValue;
}

void ConfigParser::checkIfBlockEmpty(std::string blockType) {
	if (blockType == "location") {
		if (isType(m_tokens[tokenIndex + 1], TokenType::Word) && 
				isType(m_tokens[tokenIndex + 2], TokenType::StartBlock) && 
				isType(m_tokens[tokenIndex + 3], TokenType::EndBlock))
			throw ConfigParseException("Block " + blockType + " is empty");
	}
	else if (blockType == "server") {
		if (isType(m_tokens[tokenIndex], TokenType::Word) && 
				isType(m_tokens[tokenIndex + 1], TokenType::StartBlock) && 
				isType(m_tokens[tokenIndex + 2], TokenType::EndBlock))
			throw ConfigParseException("Block " + blockType + " is empty");
	}

}

void ConfigParser::incTokenIndex(int amount) {
	if (tokenIndex + amount < m_tokens.size()) {
		tokenIndex += amount;
		return;
	}
	throw ConfigParseException("Unexpected end of file");
}

// 200, 301, 302, 400, 401, 403, 404, 405, 500, 502, 503, 504