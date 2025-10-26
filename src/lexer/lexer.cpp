#include "lexer/lexer.h"
#include "lexer.h"

using namespace rain;

mem::Pool<Token> Token::pool = mem::Pool<Token>(1000);

static Token *generate(Lexer *lexer) {
    return nullptr;
}

void Lexer::produce(int required)
{
    for (int i = 0 ; i < required ; i ++) {
        this->token_sequence.push_back(generate(this));
    }
}