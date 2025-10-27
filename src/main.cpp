#include <iostream>

#include "lexer/lexer.h"
#include "file/helper.h"

int main(int, char**){
    rain::Lexer lexer(rain::readall("./text.txt"));
    rain::Token *tok = lexer.peer();
    for (int i = 0 ; i < 21 ; i ++) {
        if (tok->type & rain::TokenType::MASK_ERROR) {
            std::cout << "Got an Error Token!" << std::endl;
        }
        else {
            std::cout << std::format("(type: {}, content: \"{}\")", tok->type, tok->content) << std::endl;
        }
        if (i != 20)
            tok = lexer.next();
    }
    return 0;
}
