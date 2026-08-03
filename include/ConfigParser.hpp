#pragma once
#include <iostream>
#include <vector>
#include <unordered_set>
#include "GlobalConfig.hpp"
#include "TokenType.hpp"
#include "Colors.hpp"

class ConfigParser {
	private:
		std::vector<Token>	m_tokens;
		std::string 				m_configPath;
		std::string 				m_buffer;
		GlobalConfig				&m_config;
		unsigned int				m_tokenIndex;
		
	public:
		ConfigParser(char *pathToConfig, GlobalConfig& config) : m_config(config), m_tokenIndex(0) {
			m_configPath = pathToConfig;
			std::cout << GREEN << "Config File Path: " << RESET << m_configPath << std::endl;
		}
		~ConfigParser() {};
		
		bool 	processConfig();
		bool 	initialFileCheck(std::fstream& file);
		void 	tokenize(std::fstream& file);
		int 	handleBraces(int index);
		int 	handleEndDirective(int index);
		int		handleWord(int index);
		int		skipComments(unsigned long index);
		void	parseTokens();
		void	parseBlock(bool isGlobal);
		void	parseDirective();
		void	parseLocationDirectives();
		void	checkAllBraces();
		void 	handleDirective();
		void	handleLocationDirective();
		void	handleListen();
		void 	handleServerName();
		void	handleRoot(int scope);
		void	handleIndex(int scope);
		void	handleBodySize(int scope);
		void	handleLocation();
		void	handleUnknown();
		void	handleAutoIndex(int scope);
		void	handleErrorPages(int scope);
		void	handleReturn();
		void	handleAllowedMethods();
		void	handleUploadStore();
		int		validateErrorCode(std::string code);
		std::string	checkURI();
		void	checkEndOfDirective(std::string directive);
		void	checkIfBlockEmpty(std::string blockType);
		void	printTokens(); // for debug only
		bool	isValidToken(const Token& token);
		bool	isType(const Token& token, TokenType expectedType);
		bool	isValue(const Token& token, const std::string& expectedValue);
		std::unordered_set<std::string>	knownDirectives = {
				"server", "listen", "server_name", "root", "index",
				"location", "methods", "autoindex", "return",
				"allowed_methods", "upload_store", "client_max_body_size"
		};
		std::unordered_set<std::string> allowedMethods = {"GET", "POST", "DELETE"};
		std::unordered_set<std::string> errorCodes = {"300", "400", "402", "403", "404", "500"};
		void 	incTokenIndex(unsigned int amount);
		void checkDuplicateLocations(std::string path);
		std::unordered_set<std::string> seenLocations;
		ServerConfig&	currentServer();
		LocationConfig& currentLocation();
		Token&	currentToken();

};