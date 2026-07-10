#pragma once
#include <iostream>

enum class TokenType {
	OpenBrace,
	ClosingBrace,
	Word,
	Semicolon,
};

struct Token {
	std::string value;
	TokenType		type;
};