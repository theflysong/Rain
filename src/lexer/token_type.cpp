#include "lexer/token_type.h"
#include "token_type.h"

using namespace rain;
    
TokenType rain::operator|(TokenType lhs, TokenType rhs) {
    return static_cast<TokenType>(static_cast<short>(lhs) | static_cast<short>(rhs));
}

TokenType rain::operator|=(TokenType &lhs, TokenType rhs) {
    lhs = static_cast<TokenType>(static_cast<short>(lhs) | static_cast<short>(rhs));
    return lhs;
}
    
bool rain::operator&(TokenType lhs, TokenType rhs) {
    return static_cast<TokenType>(static_cast<short>(lhs) & static_cast<short>(rhs)) == rhs;
}