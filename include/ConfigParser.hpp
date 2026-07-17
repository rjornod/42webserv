#pragma once
#include <iostream>
#include <vector>
#include "GlobalConfig.hpp"
#include "TokenType.hpp"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

class ConfigParser {
	private:
		std::vector<Token>	m_tokens;
		std::string 				m_configPath;
		std::string 				m_buffer;
	public:
		ConfigParser(char *pathToConfig){
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
		bool	parseTokens();
		bool	parseBlock(bool isGlobal);
		bool	parseDirective();
		bool	checkAllBraces();
};