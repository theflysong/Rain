#include <iostream>

#include "lexer/lexer.h"
#include "parser/syntax.h"
#include "file/helper.h"
#include "parser/ast_dot.h"

int main(int, char**){
    rain::Lexer lexer(rain::readall("./text.txt"));
    rain::Token *tok = lexer.peer();
    constexpr int tokcnt = 9;
    for (int i = 0 ; i < tokcnt ; i ++) {
        if (tok->type & rain::TokenType::MASK_ERROR) {
            std::cout << "Got an Error Token!" << std::endl;
        }
        else {
            std::cout << std::format("[{}](type: {}, content: \"{}\")", i + 1, tok->type, tok->content) << std::endl;
        }
        if (i < tokcnt - 1)
            tok = lexer.next();
    }

    assert(lexer.rewind(tokcnt - 1));

    auto res = rain::ExprNode::parse(lexer.token_sequence.begin(), lexer.token_sequence.end());

    if (res.success) {
        std::cout << "Parsed successfully!" << std::endl;
        std::cout << res.end - lexer.token_sequence.begin() << std::endl;
        rain::generate_ast_dot_to_file("ast.dot", res.val);
    } else {
        std::cout << "Parse failed!" << std::endl;
    }

    rain::Token::pool.cleanup();
    rain::PosInfo::pool.cleanup();

    return 0;
}
