#pragma once
#include <iostream>
#include "Colors.hpp"

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