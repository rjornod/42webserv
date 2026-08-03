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

// unsigned int  tokenIndex = 0;

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
		std::cout << BLUE << "All braces are paired\n" << RESET <<std::endl;
}
void ConfigParser::parseTokens() {
	checkAllBraces();
	// check if server block exists
	if (tokenIndex + 1 >= m_tokens.size() || !isValue(m_tokens[tokenIndex], "server")
	|| !isType(m_tokens[tokenIndex + 1], TokenType::StartBlock)) {
		throw ConfigParseException("Only server{} allowed as a global directivfffe");
	}
	std::cout << "size: " << m_tokens.size() << "\n";
	while (tokenIndex < m_tokens.size() - 1) {
		m_config.createServerConfig();
		parseBlock(true);
	}
}

void ConfigParser::parseBlock(bool isGlobal) {
	if (isGlobal) {
		if (!isValue(m_tokens[tokenIndex], "server"))
			throw ConfigParseException("Only server{} allowed as a global directiveeeee");
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
	std::cout << "Found endblock\n" << m_tokens[tokenIndex];
	if (tokenIndex + 1 < m_tokens.size() - 1)
			incTokenIndex(1);
}

void ConfigParser::parseDirective()
{
	if (isValue(m_tokens[tokenIndex], "location")) {
		if (tokenIndex + 2 >= m_tokens.size() || !isType(m_tokens[tokenIndex + 1], TokenType::Word)
			|| !isType(m_tokens[tokenIndex + 2], TokenType::StartBlock))
			throw ConfigParseException("Location directive is malformed");
		checkIfBlockEmpty("location");
		currentServer().createLocationConfig();	
		parseLocationDirectives();
	}
	else
		handleDirective();	
}

void ConfigParser::parseLocationDirectives() {
	incTokenIndex(1); 																												// skip location token
	checkDuplicateLocations(m_tokens[tokenIndex].value);
	currentLocation().setPath(m_tokens[tokenIndex].value);
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
	currentServer().setPort(port);
	checkEndOfDirective("listen");
	std::cout << BLUE << "LISTEN OK\n" << RESET;
}

void ConfigParser::handleServerName() {
	incTokenIndex(1);
	if 	(!isType(m_tokens[tokenIndex], TokenType::Word) || 
			!isValidToken(m_tokens[tokenIndex]))
		throw ConfigParseException("server_name directive is missing argument");
	currentServer().setServerName(m_tokens[tokenIndex].value);
	checkEndOfDirective("server_name");
	std::cout << BLUE << "NAME OK\n" << RESET;
}

void ConfigParser::handleRoot(int scope) {
	incTokenIndex(1);
	if 	(!isType(m_tokens[tokenIndex], TokenType::Word) || 
			!isValidToken(m_tokens[tokenIndex]))
		throw ConfigParseException("root directive is missing argument");
	if (scope == GLOBAL)
		currentServer().setRoot(m_tokens[tokenIndex].value);
	else 
		currentLocation().setRoot(m_tokens[tokenIndex].value);
	checkEndOfDirective("root");
	std::cout << BLUE << "ROOT OK\n" << RESET;
}

void ConfigParser::handleIndex(int scope) {
	incTokenIndex(1);
	if (!isType(m_tokens[tokenIndex], TokenType::Word))
		throw ConfigParseException("Index directive is malformed");
	// check if the current token is a Word and is not the same as a known directive
	while (tokenIndex < m_tokens.size() && 
				isValidToken(m_tokens[tokenIndex])) {
		if (scope == GLOBAL)
			currentServer().setIndex(m_tokens[tokenIndex].value);
		else
			currentLocation().setIndex(m_tokens[tokenIndex].value);
		incTokenIndex(1);
	}
	if (!isType(m_tokens[tokenIndex], TokenType::EndDirective)) 
		throw ConfigParseException("Index directive is missing a semicolon");
	std::cout << BLUE << "INDEX OK\n" << RESET;
}

void ConfigParser::handleBodySize(int scope) {
	incTokenIndex(1);
	if (!validateDigits(m_tokens[tokenIndex].value))
		throw ConfigParseException("client_max_body_size argument is not a valid integer");
	int bodySize = std::stoi(m_tokens[tokenIndex].value);
	if (scope == GLOBAL)
		currentServer().setBodySize(bodySize);
	else
		currentLocation().setBodySize(bodySize);
	checkEndOfDirective("client_max_body_size");
	std::cout << BLUE << "CLIENTMAXBODYSIZE OK\n" << RESET;
}

void ConfigParser::handleUnknown() {
	throw ConfigParseException("Unknown directive detected: " + m_tokens[tokenIndex].value);
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
		currentServer().setAutoIndex(true);
	else if (m_tokens[tokenIndex].value == "off" && scope == GLOBAL)
		currentServer().setAutoIndex(false);
	else if (m_tokens[tokenIndex].value == "on" && scope == LOCATION)
		currentLocation().setAutoIndex(true);
	else if (m_tokens[tokenIndex].value == "off" && scope == LOCATION)
		currentLocation().setAutoIndex(false);

	checkEndOfDirective("autoindex");
	std::cout << BLUE << "AUTOINDEX OK\n" << RESET;

}

int ConfigParser::validateErrorCode(std::string errorCode) {
	
	if (errorCode.size() != 3)
		throw ConfigParseException("Error code in error_pages is invalid: " + errorCode );
	for (unsigned long i = 0; i < errorCode.size(); i++) {
		if (!isdigit(errorCode[i]))
			throw ConfigParseException("Error code must contain only digits: " + errorCode);
	}
	int code = std::stoi(errorCode);
	if (!errorCodes.count(m_tokens[tokenIndex].value))
		throw ConfigParseException("Error code not allowed: " + errorCode);
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
			currentServer().setErrorPages(validateErrorCode(m_tokens[tokenIndex].value), path);
		else if (scope == LOCATION)
			currentLocation().setErrorPages(validateErrorCode(m_tokens[tokenIndex].value), path);
		incTokenIndex(1);
	}
	incTokenIndex(1); // TO DO: handle path
	if (!isType(m_tokens[tokenIndex], TokenType::EndDirective)) 
		throw ConfigParseException("error_pages directive is missing a semicolon");
	std::cout << BLUE << "ERRORPAGES OK\n" << RESET;
}
/**
 * Directive: return
 * Syntax: return code path;
 * Must always have 2 arguments;
 **/
void ConfigParser::handleReturn() {
	incTokenIndex(1);
	if (!isType(m_tokens[tokenIndex], TokenType::Word) || 
			!isType(m_tokens[tokenIndex + 1], TokenType::Word) ||
			!isValidToken(m_tokens[tokenIndex]))
		throw ConfigParseException("return directive is malformed");
	int code = validateErrorCode(m_tokens[tokenIndex].value);
	incTokenIndex(1);
	std::string errorPath = m_tokens[tokenIndex].value;
	currentLocation().setReturn(code, errorPath);
	checkEndOfDirective("return");
	std::cout << BLUE << "RETURN OK\n" << RESET;
}

/**
 * Directive: allowed_methods
 * Syntax: allowed_methods method [method method];
 * At least one method, max 3;
 * Allowed methods are POST, GET, DELETE;
 */
void ConfigParser::handleAllowedMethods() {
	incTokenIndex(1);
	std::unordered_set<std::string> seen;																												// store every method to account for duplicates
	int methodCount = 0;
	if (!isType(m_tokens[tokenIndex], TokenType::Word) || !isValidToken(m_tokens[tokenIndex]))
		throw ConfigParseException("allowed_methods directive is malformed");
	currentLocation().clearDefaultMethods();
	while (!isType(m_tokens[tokenIndex], TokenType::EndDirective)) {
		if (methodCount>= 3)
			throw ConfigParseException("Too many arguments for allowed_methods. Max 3 allowed.");
		if (!seen.insert(m_tokens[tokenIndex].value).second)																								// check if method is duplicate
			throw ConfigParseException("Duplicate methods are not allowed: " + m_tokens[tokenIndex].value);
		if (isType(m_tokens[tokenIndex], TokenType::Word) && 
				allowedMethods.count(m_tokens[tokenIndex].value))
			currentLocation().setAllowedMethod(m_tokens[tokenIndex].value);
		else
			throw ConfigParseException("Method incorrect. Only GET, POST and DELETE allowed: " + m_tokens[tokenIndex].value);
		methodCount++;
		incTokenIndex(1);
	}
	if (!isType(m_tokens[tokenIndex], TokenType::EndDirective)) 
		throw ConfigParseException("allowed_methods directive is missing a semicolon");
	std::cout << BLUE << "ALLOWED_METHODS OK" << RESET << std::endl;
}

/**
 * Directive: upload_store
 * Syntax: upload_store <directory>;
 * Only one argument allowed;
 */
void ConfigParser::handleUploadStore() {
	incTokenIndex(1);
	if (!isType(m_tokens[tokenIndex], TokenType::Word) || 
			!isValidToken(m_tokens[tokenIndex]) ||
			!isType(m_tokens[tokenIndex + 1], TokenType::EndDirective)
			)
		throw ConfigParseException("upload_store directive is malformed");
	currentLocation().setUploadStore(m_tokens[tokenIndex].value);
	checkEndOfDirective("upload_store");
	std::cout << BLUE << "UPLOAD_STORE OK\n" << RESET;
}

void ConfigParser::handleLocation() {
	incTokenIndex(1);
	parseBlock(false);
	std::cout << BLUE << "LOCATION OK\n" << RESET;
}

void ConfigParser::handleLocationDirective() {
	switch(locationDirectiveFromString(m_tokens[tokenIndex].value)) {
		case LocationDirectiveType::Root:
			handleRoot(LOCATION);
			break;
		case LocationDirectiveType::AutoIndex:
			handleAutoIndex(LOCATION);
			break;
		case LocationDirectiveType::MaxBodySize:
			handleBodySize(LOCATION);
			break;
		case LocationDirectiveType::Index:
			handleIndex(LOCATION);
			break;
		case LocationDirectiveType::ErrorPage:
			handleErrorPages(LOCATION);
			break;
		case LocationDirectiveType::UploadStore:
			handleUploadStore();
			break;
		case LocationDirectiveType::AllowedMethods:
			handleAllowedMethods();
			break;
		case LocationDirectiveType::Return:
			handleReturn();
			break;
		case LocationDirectiveType::Unknown:
			handleUnknown();
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
			handleAutoIndex(GLOBAL);
			break;
		case DirectiveType::MaxBodySize:
			handleBodySize(GLOBAL);
			break;
		case DirectiveType::Unknown:
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

void ConfigParser::checkDuplicateLocations(std::string path) {
	std::cout << "path is: " << path <<"\n";
	if (!seenLocations.insert(path).second)
		throw ConfigParseException("Duplicate locations not allowed: " + path);
}

void ConfigParser::incTokenIndex(unsigned int amount) {
	if (m_tokenIndex + amount < m_tokens.size()) {
		m_tokenIndex += amount;
		return;
	}
	throw ConfigParseException("Unexpected end of file");
}

/**
 * Small helper to get the current server object
 */
ServerConfig& ConfigParser::currentServer() {
	if (m_config.getServerConfigs().empty())
		throw ConfigParseException("No server config available");
	return m_config.getServerConfigs().back();
}

/**
 * Small helper to get the current location object
 */
LocationConfig& ConfigParser::currentLocation() {
	if (m_config.getServerConfigs().back().getLocationConfigs().empty())
		throw ConfigParseException("No location config available");
	return m_config.getServerConfigs().back().getLocationConfigs().back();
}

/**
 * Small helper to get the current token
 */
Token& ConfigParser::currentToken() {
	return (m_tokens[m_tokenIndex]);
}

// 200, 301, 302, 400, 401, 403, 404, 405, 500, 502, 503, 504