#include <iostream>

#include "lexer/lexer.h"
#include "parser/syntax.h"
#include "file/helper.h"
#include "parser/syntax_dot.h"
#include "codegen/codegen.h"

int main(int, char**){
    rain::Lexer lexer(rain::readall("./text.txt"));

    lexer.lex_all();
    lexer.token_sequence.push_back(new rain::Token(rain::TokenType::ENDMARK, "$", rain::makepos(lexer.pos)));
    std::cout << "Tokens:" << std::endl;

    for (const auto &tok : lexer.token_sequence) {
        std::cout << tok->repr() << std::endl;
    }

    auto res = rain::ProgramNode::parse(lexer.token_sequence.begin(), lexer.token_sequence.end());
    auto *program_node = res.val;

    if (res.success) {
        std::cout << "Parsed successfully!" << std::endl;
        std::cout << res.end - lexer.token_sequence.begin() << std::endl;
        rain::generate_ast_dot_to_file("ast.dot", program_node);
    } else {
        std::cout << "Parse failed!" << std::endl;
    }

    rain::SymbolTable symtab;
    rain::CodeGenContext ctx(symtab);

    bool result = rain::gen_program(program_node, ctx);
    if (result) {
        std::cout << "Generated program successfully!" << std::endl;
    } else {
        std::cout << "Program generation failed!" << std::endl;
    }

    rain::IASTNode::pool.cleanup();
    rain::Token::pool.cleanup();
    rain::PosInfo::pool.cleanup();

    return 0;
}
