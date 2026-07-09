#pragma once
#include <iostream>
#include <vector>
#include "GlobalConfig.hpp"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

class ConfigTokenizer {
	private:
		std::string m_configPath;
		std::vector<std::string> tokens;
	public:
		ConfigTokenizer(char *pathToConfig){
			m_configPath = pathToConfig;
			std::cout << GREEN << "Config File Path: " << RESET << m_configPath << std::endl;
		} 
			
		~ConfigTokenizer() {};
		bool parseFile();
		bool initialFileCheck(std::fstream& file);
		void tokenize(std::fstream& file);
};