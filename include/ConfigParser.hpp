#pragma once
#include <iostream>
#include <vector>
#include "GlobalConfig.hpp"
#include "TokenType.hpp"
#include "Colors.hpp"

class ConfigParser {
	private:
		std::vector<Token>	m_tokens;
		std::string 				m_configPath;
		std::string 				m_buffer;
		GlobalConfig				&m_config;
	public:
		ConfigParser(char *pathToConfig, GlobalConfig& config) : m_config(config) {
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
		int		skipComments(int index);
		void	parseTokens();
		void	parseBlock(bool isGlobal);
		void	parseDirective();
		void	checkAllBraces();
		void 	handleDirective();
		void	handleListen();
		void 	handleServerName();
		void	handleRoot();
		void	handleIndex();
		void	handleBodySize();
};