#pragma once
#include <iostream>
#define MAGENTA "\x1b[35m"
#define RESET   "\x1b[0m"

enum class TokenType {
	Word,
	StartBlock,
	EndBlock,
	EndDirective,
};

struct Token {
	TokenType		type;
	std::string value;
};

constexpr const char* to_string(TokenType type)
{
    switch (type) {
      case TokenType::Word: return "Word";
      case TokenType::StartBlock:   return "StartBlock";
      case TokenType::EndBlock: return "EndBlock";
			case TokenType::EndDirective: return "EndDirective";
    }
    return "ERROR";
}

std::ostream& operator<<(std::ostream& out, const TokenType& type);
std::ostream& operator<<(std::ostream& out, const Token& token);