#include "../include/TokenType.hpp"

std::ostream& operator<<(std::ostream& out, const TokenType& type)
{
    return out << to_string(type);
}

std::ostream& operator<<(std::ostream& out, const Token& token)
{
    return out << MAGENTA << "type: " << RESET << token.type << "," << MAGENTA << " value: "<< RESET << "'" << token.value << "'";
}