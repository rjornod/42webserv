#include <fstream>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>  
#include "ConfigParser.hpp"
#include "ConfigParseException.hpp"
#include "TokenType.hpp"
#include "DirectiveType.hpp"
#include "LocationDirectiveType.hpp"

// macros that define the scope of different directives (error_page can be global (in a server block) or inside a location block)
#define GLOBAL 1
#define LOCAL 0

/**
 * This is the main function for the config parsing;
 * It will catch exceptions thrown from functions that handle all the parsing;
 */
bool ConfigParser::processConfig() {
	
	std::ifstream file(m_configPath);
	try {
		initialFileCheck(file);
		tokenize(file);
	}
	catch(const ConfigParseException& e) {
		std::cerr << RED << "File Error: " << RESET << e.what() << '\n';
		return false;
	}
	try {
		parseTokens();
	}
	catch (const ConfigParseException& e) {
		std::cerr << RED << "Config Error: " << RESET << e.what() << std::endl;
		return false;
	}
	catch (const std::out_of_range& e) {
		std::cerr << RED << "Out of range error: " << RESET << e.what() << '\n';
		return false;
	}
	return true;
}

/**
 * Function that does basic checks on the file before the parsing starts;
 * Checks if the file exists and is not empty;
 * Perhaps more can be added still;
 */
void ConfigParser::initialFileCheck(std::ifstream& file) {
	if (m_configPath.substr(m_configPath.size() - 5) != ".conf")
		throw ConfigParseException("Config file must be a '.conf' file\n");
	if (!file.is_open()) 
		throw ConfigParseException("Can't open config file. Make sure it's valid.");
	bool isEmpty = file.peek() == EOF;
	if (isEmpty)
		throw ConfigParseException("Config file is empty. Make sure you use a valid config file");
}

/**
 * Function that atually tokenizes the config file;
 * It goes through every string in the file, ignores any comments,
 * and checks what kind of token that string would be;
 */
void ConfigParser::tokenize(std::ifstream& file) {
	while (std::getline(file, m_buffer)) {
		if (m_buffer.empty())																	// skip empty lines
			continue;
		unsigned long i = 0;
		while (i < m_buffer.size()) {
			if (m_buffer[i] == '#') {
				i = skipComments(i);
				continue;
			}
			if (isspace(m_buffer[i])) {
				i++;
				continue;
			}
			if (m_buffer[i] == ';') {
				i = handleEndDirective(i);
				continue;
			}
			if ((m_buffer[i] == '{' || m_buffer[i] == '}')) {
				i = handleBraces(i);
				continue;
			}
			if (!isspace(m_buffer[i]) && m_buffer[i] !=';') {
				i = handleWord(i);
				continue;
			}
		}
	}
	if (m_tokens.empty())
		throw ConfigParseException("File doesn't have any server block");
}

/**
 * Function that detects braces and places them on the token list;
 */
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

/**
 * Function that detects semicolons and places them on the token list;
 */
int ConfigParser::handleEndDirective(int i) {
	if (m_buffer[i] == ';') {
		m_tokens.emplace_back(Token{TokenType::EndDirective, std::string(1, m_buffer[i])});
		i++;
	}
	return i;
}

/**
 * Function that detects words (no difference in what kind of word it is) and places them on the token list;
 */
int ConfigParser::handleWord(int i) {
	int tokenStart = i;
	while (!isspace(m_buffer[i]) && m_buffer[i] != ';'  && m_buffer[i] !='{' && m_buffer[i] !='}') 
		++i;
	m_tokens.emplace_back(Token{TokenType::Word, m_buffer.substr(tokenStart, i - tokenStart)});
	return i;
}

/**
 * Function that ignores comments on the config file, regardless of where they are;
 */
int ConfigParser::skipComments(unsigned long i) {
	if (m_buffer[i] == '#') {
		while (i < m_buffer.size())
			i++;
	}
	return i;
}

/**
 * Function that simply checks if there are any missing braces in the config file;
 * Doesn't check if braces are all placed correctly, only if every open brace has a closing brace;
 */ 
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
}

/**
 * This is where the token parsing starts;
 * First I check if the first token is "server" because that's the only valid global block allowed;
 * If it's not, the config file is wrong;
 * If it is then the parsing starts by checking each individual directive of the server;
 */
void ConfigParser::parseTokens() {
	checkAllBraces();
	// check if server block exists
	if (m_tokenIndex + 1 >= m_tokens.size() || !isType(currentToken(), TokenType::Word)
	|| !isType(currentTokenPlus(1), TokenType::StartBlock))
		throw ConfigParseException("Global block is malformed");
	while (m_tokenIndex < m_tokens.size() - 1) {
		m_config.createServerConfig();
		parseBlock(true);
		currentServer().checkMandatoryDirectives();
	}
}

/**
 * Function that will be called a when a block (server or location) is found;
 * It will check that is formed properly and then call parseDirective() to parse
 * all the directives inside the block;
 */
void ConfigParser::parseBlock(bool isGlobal) {
	if (isGlobal) {
		if (!isValue(currentToken(), "server"))
			throw ConfigParseException("Only server{} allowed as a global directive");
	}
	else if (!isValue(currentToken(), "location"))
		throw ConfigParseException("Directive in server block is malformed " + currentToken().value);
	checkIfBlockEmpty("server");			
	incTokenIndex(2);																										// skip server or location token and StartBlock token
	while (!isType(currentToken(), TokenType::EndBlock)) {
		parseDirective();
		incTokenIndex(1);		
	}
	if (m_tokenIndex + 1 < m_tokens.size() - 1)
		incTokenIndex(1);
}

void ConfigParser::parseDirective()
{
	if (isValue(currentToken(), "location")) {
		if (m_tokenIndex + 2 >= m_tokens.size() || !isType(currentTokenPlus(1),  TokenType::Word)
			|| !isType(currentTokenPlus(2), TokenType::StartBlock))
			throw ConfigParseException("Location directive is malformed");
		checkIfBlockEmpty("location");
		currentServer().createLocationConfig();	
		parseLocationDirectives();
	}
	else
		handleDirective();	
}

/**
 * Function gets called when a location block is reached;
 * It will check every line for the different location directives until a semicolon is found;
 */
void ConfigParser::parseLocationDirectives() {
	incTokenIndex(1); 																												// skip location token
	currentServer().checkDuplicateLocations(currentToken().value);
	currentLocation().setPath(currentToken().value);
	incTokenIndex(2);
	while (!isType(currentToken(), TokenType::EndDirective)) {
		if (isType(currentToken(), TokenType::EndBlock))
			return;
		handleLocationDirective();
		incTokenIndex(1);
	}
	incTokenIndex(1);
}

/**
 * This function gets called after checking that the needed arguments are present;
 * This will check if there are more arguments after that and if there is a semicolon present
 */
void ConfigParser::checkEndOfDirective(std::string directive) {
	incTokenIndex(1);
	if 	(isValidToken(currentToken()))
	 	throw ConfigParseException(directive + " directive has too many arguments");
	if (!isType(currentToken(), TokenType::EndDirective))
		throw ConfigParseException(directive + " directive is missing a semicolon");
}

/**
 * Small helper functon that checks if the digits passed are all actually digits;
 * Used for the listen and client_max_body directives;
 */
static bool validateDigits(std::string_view digits) {
	for (unsigned long i = 0; i < digits.size(); i++) {
		if (!isdigit(digits[i]))
			return false;
	}
	return true;
}

/**
 * Directive: listen
 * Syntax: listen <port>;
 * Only one argument allowed;
 * Argument needs to be a valid integer between 1024 and 65535;
 * Ports below 1024 need sudo requirement and above 65535 don't exist;
 */
void ConfigParser::handleListen() {
	incTokenIndex(1);
	if 	(!isType(currentToken(), TokenType::Word) || 
			!isValidToken(currentToken()))
		throw ConfigParseException("listen directive is missing argument");
	if (currentToken().value.size() > 5)
		throw ConfigParseException("Port '" + currentToken().value + "' is not valid. Must be an integer with max value of 65535");
	if (!validateDigits(currentToken().value))
		throw ConfigParseException("Port argument is not a valid integer");
	int port = std::stoi(currentToken().value);
	if (port < 1024 || port > 65535)
		throw ConfigParseException("Port has to be a value between 1024 and 65535");
	checkDuplicatePorts(port);
	currentServer().setPort(port);
	checkEndOfDirective("listen");
	currentServer().seenDirective("listen");
}

/**
 * Directive: server_name
 * Syntax: server_name <name>;
 * Must only have 1 argument;
 * TO DO: make sure the server name is unique, reject duplicates
 */
void ConfigParser::handleServerName() {
	incTokenIndex(1);
	if 	(!isType(currentToken(), TokenType::Word) || 
			!isValidToken(currentToken()))
		throw ConfigParseException("server_name directive is missing argument");
	checkDuplicateServerNames(currentToken().value);
	currentServer().setServerName(currentToken().value);
	checkEndOfDirective("server_name");
	currentServer().seenDirective("server_name");
}

/**
 * Directive: root
 * Syntax: root <path>;
 * Must only have one argument
 * 
 * TO DO: not sure if I validate the path here or later..
 */
void ConfigParser::handleRoot(int scope) {
	incTokenIndex(1);
	if 	(!isType(currentToken(), TokenType::Word) || 
			!isValidToken(currentToken()))
		throw ConfigParseException("root directive is missing argument");
	if (scope == GLOBAL)
		currentServer().setRoot(currentToken().value);
	else 
		currentLocation().setRoot(currentToken().value);
	checkEndOfDirective("root");
	currentServer().seenDirective("root");
}

/**
 * Directive: index
 * Syntax: index <path> [<path> <path> ...];
 * index must have at least 1 argument but multiple arguments are also allowed
 */
void ConfigParser::handleIndex(int scope) {
	incTokenIndex(1);
	if (!isType(currentToken(), TokenType::Word))
		throw ConfigParseException("Index directive is malformed");
	while (m_tokenIndex < m_tokens.size() && 
				isValidToken(currentToken())) {
		if (scope == GLOBAL)
			currentServer().setIndex(currentToken().value);
		else
			currentLocation().setIndex(currentToken().value);
		incTokenIndex(1);
	}
	if (!isType(currentToken(), TokenType::EndDirective)) 
		throw ConfigParseException("Index directive is missing a semicolon");
	currentServer().seenDirective("index");
}

/**
 * Directive: client_max_body_size
 * Syntax: client_max_body_size <amount>;
 * Must have only 1 argument and must be a valid number;
 */
void ConfigParser::handleBodySize(int scope) {
	incTokenIndex(1);
	if (!validateDigits(currentToken().value))
		throw ConfigParseException("client_max_body_size argument is not a valid integer");
	// check if the value is within int max
	// unsigned long temp = std::stoul(currentToken().value);
	// if (std::numeric_limits<int>::max() < temp)
	// 	throw ConfigParseException("client_max_body_size is too big. max value is 2147483647");
	int bodySize = customStoi(currentToken().value, "client_max_body_size is too big. Max value is 2147483647");
	// int bodySize = std::stoi(currentToken().value);
	if (scope == GLOBAL)
		currentServer().setBodySize(bodySize);
	else
		currentLocation().setBodySize(bodySize);
	checkEndOfDirective("client_max_body_size");
	currentServer().seenDirective("client_max_body_size");
}

/**
 * Anything that is not a recognized directive (for server or location) will trigger
 * this case;
 */
void ConfigParser::handleUnknown() {
	throw ConfigParseException("Unknown directive detected: " + currentToken().value);
}

/**
 * Directive: autoindex
 * Syntax: autoindex <state>;
 * Must have only 1 argument;
 * State can be only 1 of 2 values: <on> or <off>;
 * Anything else is considered an error;
 */
void ConfigParser::handleAutoIndex(int scope) {
	incTokenIndex(1);
	if 	(!isType(currentToken(), TokenType::Word) || 
			!isValidToken(currentToken()))
		throw ConfigParseException("autoindex directive is missing argument");
	if (currentToken().value != "on" && currentToken().value != "off")
		throw ConfigParseException("autoindex argument is not correct. Only 'on' or 'off' allowed");
	if (currentToken().value == "on" && scope == GLOBAL)
		currentServer().setAutoIndex(true);
	else if (currentToken().value == "off" && scope == GLOBAL)
		currentServer().setAutoIndex(false);
	else if (currentToken().value == "on" && scope == LOCAL)
		currentLocation().setAutoIndex(true);
	else if (currentToken().value == "off" && scope == LOCAL)
		currentLocation().setAutoIndex(false);
	checkEndOfDirective("autoindex");
	currentServer().seenDirective("autoindex");
}

/**
 * This function gets called to validate the error codes;
 * It will check if the string is all digits, if the string size is bigger than 3 (all error codes are 3 digits)
 * and if the error code is allowed;
 * 
 */
int ConfigParser::validateErrorCode(std::string errorCode) {
	
	if (errorCode.size() != 3)
		throw ConfigParseException("Error code in error_page is invalid: " + errorCode );
	validateDigits(errorCode);
	int code = std::stoi(errorCode);
	if (!errorCodes.count(currentToken().value))
		throw ConfigParseException("Error code not allowed: " + errorCode);
	return code;
}

/**
 * Function check if the URI is absolute;
 * Used in error_page directive;
 * 
 * TO DO: Not sure if it's the job of the parser to validate this or if
 * I should just accept whatever is there as long as the token is a word and let
 * the execution figure it out later. If the path is not correct then I guess
 * a default error page would be shown to the user (404 or something like that)
 */
std::string ConfigParser::findURI() {
	unsigned long i = m_tokenIndex;
	while (!isType(m_tokens[i + 1], TokenType::EndDirective) && isValidToken(m_tokens[i + 1]))	
		i++;
	if (!isType(m_tokens[i + 1], TokenType::EndDirective))
		throw ConfigParseException("error_page directive is missing a semicolon");
	return m_tokens[i].value;
}

/**
 * Directive: error_page
 * Syntax: error_page <error_code> [<error_code> <error_code> ....] <path>;
 * Has at least 2 arguments: at least one error code and a path;
 * Error codes must be valid and there can be multiple in a row;
 * Last argument is always a path;
 */
void ConfigParser::handleErrorPages(int scope) {
	incTokenIndex(1);
	if (!isType(currentToken(), TokenType::Word) || 
			!isType(currentTokenPlus(1), TokenType::Word) ||
			!isValidToken(currentToken()) ||
			!isValidToken(currentTokenPlus(1)))
		throw ConfigParseException("error_page directive is malformed");
	std::string path = findURI();
	while (m_tokenIndex < m_tokens.size() && 
				isValidToken(currentToken()) &&
				!isType(currentTokenPlus(1), TokenType::EndDirective) ) {
		if (scope == GLOBAL)
			currentServer().setErrorPages(validateErrorCode(currentToken().value), path);
		else if (scope == LOCAL)
			currentLocation().setErrorPages(validateErrorCode(currentToken().value), path);
		incTokenIndex(1);
	}
	incTokenIndex(1);
	currentServer().seenDirective("error_page");
}
/**
 * Directive: return
 * Syntax: return <code> <path>;
 * Must always have 2 arguments;
 * First is always a valid error code and second is always a path
 **/
void ConfigParser::handleReturn() {
	incTokenIndex(1);
	if (!isType(currentToken(), TokenType::Word) || 
			!isType(currentTokenPlus(1), TokenType::Word) ||
			!isValidToken(currentToken()))
		throw ConfigParseException("return directive is malformed");
	int code = validateErrorCode(currentToken().value);
	incTokenIndex(1);
	std::string errorPath = currentToken().value;
	currentLocation().setReturn(code, errorPath);
	checkEndOfDirective("return");
}

/**
 * Directive: allowed_methods
 * Syntax: allowed_methods <method> [<method> <method>];
 * At least one method, max 3;
 * Allowed methods are POST, GET, DELETE;
 */
void ConfigParser::handleAllowedMethods() {
	incTokenIndex(1);
	std::unordered_set<std::string> seen;																												// store every method to account for duplicates
	int methodCount = 0;
	if (isType(currentToken(), TokenType::EndDirective))
		throw ConfigParseException("allowed_methods directive is missing an argument");
	if (!isType(currentToken(), TokenType::Word) || !isValidToken(currentToken()))
		throw ConfigParseException("allowed_methods directive is malformed");
	currentLocation().clearDefaultMethods();
	while (!isType(currentToken(), TokenType::EndDirective)) {
		if (methodCount>= 3)
			throw ConfigParseException("Too many arguments for allowed_methods. Max 3 allowed.");
		if (!seen.emplace(currentToken().value).second)																								// check if method is duplicate
			throw ConfigParseException("Duplicate methods are not allowed: " + currentToken().value);
		if (isType(currentToken(), TokenType::Word) && knownDirectives.count(m_tokens[m_tokenIndex + 1].value))
			throw ConfigParseException("Allowed methods directive is missing a semicolon");
		if (isType(currentToken(), TokenType::Word) && 
			allowedMethods.count(currentToken().value))
			currentLocation().setAllowedMethod(currentToken().value);
		else
			throw ConfigParseException("Method incorrect. Only GET, POST and DELETE allowed: " + currentToken().value);
		methodCount++;
		incTokenIndex(1);
	}
	if (!isType(currentToken(), TokenType::EndDirective)) 
		throw ConfigParseException("allowed_methods directive is missing a semicolon");
}

/**
 * Directive: upload_store
 * Syntax: upload_store <directory>;
 * Only one argument allowed;
 */
void ConfigParser::handleUploadStore() {
	incTokenIndex(1);
	if (!isType(currentToken(), TokenType::Word) || 
			!isValidToken(currentToken()) ||
			!isType(currentTokenPlus(1), TokenType::EndDirective)
			)
		throw ConfigParseException("upload_store directive is malformed");
	currentLocation().setUploadStore(currentToken().value);
	checkEndOfDirective("upload_store");
}

/**
 * Directive: cgi_handler
 * Syntax: cgi_handler <extension> <path to interpreter>;
 * Must always have two arguments
 * Extension must start with a dot '.' and might be restricted to only certain extensions 
 * For now only php is accepted as a placeholder
 * The path will be checked during execution (?) 
 */
void ConfigParser::handleCGI(int scope) {
	incTokenIndex(1);
	if (!isType(currentToken(), TokenType::Word) || 
			!isValidToken(currentToken()) ||
			!isValidToken(currentTokenPlus(1)) ||
			!isType(currentTokenPlus(1), TokenType::Word) ||
			!isType(currentTokenPlus(2), TokenType::EndDirective)
			)
		throw ConfigParseException("cgi_handler directive is malformed");
	checkCgiExtension(currentToken().value);
	if (scope == GLOBAL)
		currentServer().setCgiHandler(currentToken().value, currentTokenPlus(1).value);
	else if (scope == LOCAL)
		currentLocation().setCgiHandler(currentToken().value, currentTokenPlus(1).value);
	incTokenIndex(1);
	checkEndOfDirective("cgi_handler");
}

/**
 * Location is handled by calling parseBlock with false (not global block) as an argument
 * parseBlock will then run and check the directives specific for location blocks
 */
void ConfigParser::handleLocation() {
	incTokenIndex(1);
	parseBlock(false);
}

/**
 * Switch case for all the different location directives allowed
 */
void ConfigParser::handleLocationDirective() {
	switch(locationDirectiveFromString(currentToken().value)) {
		case LocationDirectiveType::ROOT:
			handleRoot(LOCAL);
			break;
		case LocationDirectiveType::AUTOINDEX:
			handleAutoIndex(LOCAL);
			break;
		case LocationDirectiveType::MAXBODYSIZE:
			handleBodySize(LOCAL);
			break;
		case LocationDirectiveType::INDEX:
			handleIndex(LOCAL);
			break;
		case LocationDirectiveType::ERRORPAGE:
			handleErrorPages(LOCAL);
			break;
		case LocationDirectiveType::UPLOADSTORE:
			handleUploadStore();
			break;
		case LocationDirectiveType::ALLOWEDMETHODS:
			handleAllowedMethods();
			break;
		case LocationDirectiveType::RETURN:
			handleReturn();
			break;
		case LocationDirectiveType::CGIHANDLER:
			handleCGI(LOCAL);
			break;
		case LocationDirectiveType::UNKNOWN:
			handleUnknown();
			break;
	}
}

/**
 * Switch case with all the different global directives allowed
 */
void ConfigParser::handleDirective() {
	switch(directiveFromString(currentToken().value)) {
		case DirectiveType::LISTEN:
			handleListen();
			break;
		case DirectiveType::NAME:
			handleServerName();
			break;
		case DirectiveType::ROOT:
			handleRoot(GLOBAL);
			break;
		case DirectiveType::INDEX:  
			handleIndex(GLOBAL);
			break;
		case DirectiveType::ERRORPAGE:
			handleErrorPages(GLOBAL);
			break;
		case DirectiveType::LOCATION:
			handleLocation();
			break;
		case DirectiveType::AUTOINDEX:
			handleAutoIndex(GLOBAL);
			break;
		case DirectiveType::MAXBODYSIZE:
			handleBodySize(GLOBAL);
			break;
		case DirectiveType::CGIHANDLER:
			handleCGI(GLOBAL);
			break;
		case DirectiveType::UNKNOWN:
			handleUnknown();
			break;
	}
}

/**
 * Small function to check the extension of a cgi
 */
void ConfigParser::checkCgiExtension(std::string extension) {
	int i = 0;
	if (extension.size() < 2 || extension.size() > 4)
		throw ConfigParseException("Extension argument of cgi_handler is not a proper extension");
	if (i < extension.size() && extension[i] != '.')
		throw ConfigParseException("Extension argument of cgi_handler must start with a dot '.'");
	i++;
	// this would then check if the extension matches our handled ones. if theres a lot I could make a dictionary to check them all
	// for now only accepts php as a placeholder
	if (extension.substr(i) != "php")							
		throw ConfigParseException("Extension of cgi_handler not recognized");
}

/**
 * Small helper function to check if a token is valid;
 * To be valid it needs to be a word and not a know directive like <index> or <error_page>;
 * This helps prevent the case where there are missing semicolons but introduces a case where
 * you can't have an argument be called the same as a known directive; 
 */
bool ConfigParser::isValidToken(const Token& token) {
	return 	isType(token, TokenType::Word) && 
					!knownDirectives.count(token.value);
}

/**
 * Bool function that checks if a token is what is expected to be;
 * Used in pretty much every function for validation purposes; 
 */
bool	ConfigParser::isType(const Token& token, TokenType expectedType) {
	return token.type == expectedType;
}

/**
 * Bool function that checks if the value of a token is what is expected to be;
 */
bool	ConfigParser::isValue(const Token& token, const std::string& expectedValue) {
	return token.value == expectedValue;
}

/**
 * Function checks if a block (server or location) is empty;
 * To be empty it must start be named correctly (in locations case it must have a path after the name)
 * and the next 2 tokens should be StartBlock and EndBlock;
 */
void ConfigParser::checkIfBlockEmpty(std::string blockType) {
	if (blockType == "location") {
		if (isType(currentTokenPlus(1), TokenType::Word) && 
				isType(currentTokenPlus(2), TokenType::StartBlock) && 
				isType(currentTokenPlus(3), TokenType::EndBlock))
			throw ConfigParseException("Block " + blockType + " is empty");
	}
	else if (blockType == "server") {
		if (isType(currentToken(), TokenType::Word) && 
				isType(currentTokenPlus(1), TokenType::StartBlock) && 
				isType(currentTokenPlus(2), TokenType::EndBlock))
			throw ConfigParseException("Block " + blockType + " is empty");
	}
}

void ConfigParser::checkDuplicateServerNames(const std::string& name) {
	if (!m_seenServerNames.emplace(name).second)
		throw ConfigParseException("Duplicate server names are not allowed for different servers: " + name);
}

void ConfigParser::checkDuplicatePorts(int port) {
	if (!m_seenPorts.emplace(port).second)
		throw ConfigParseException("Duplicate listens on the same port is not allowed ");
} 

/**
	 * Small helper function that first checks if the amount we want to increment
	 * tokenIndex is still in bounds and increments it if it is;
	 * Used extensively as a way to validate every token;
	 */
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
	if (m_tokenIndex < m_tokens.size())
		return (m_tokens[m_tokenIndex]);
	throw ConfigParseException("Token index is out of range");
}

/**
 * Small helper to get a specific token
 */
Token& ConfigParser::currentTokenPlus(unsigned int amount) {
	return (m_tokens[m_tokenIndex + amount]);
}

/**
 * Funtion wrapper that allows to catch an stoi exception and throw another exception
 * with a custom message
 */
int ConfigParser::customStoi(const std::string& value, const std::string& message) {
	try {
		int temp = std::stoi(value);
		return temp;
	}
	catch (const std::out_of_range& e) {
		throw ConfigParseException(message);
	}
}

// DEBUG FUNCTIONS
void ConfigParser::printTokens() {
	std::cout << BLUE << "----Printing tokens vector---" << RESET << std::endl;
	for (unsigned long i = 0; i < m_tokens.size(); i++) {
		std::cout << i << " - " <<  m_tokens[i] << std::endl;
	}
		std::cout << BLUE << "-----------------------------" << RESET << std::endl;
}
