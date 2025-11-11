#include <iostream>

#include "lexer/lexer.h"
#include "parser/syntax.h"
#include "file/helper.h"
#include "parser/syntax_dot.h"
#include "codegen/compdag.h"
#include "codegen/daggen.h"
#include "codegen/exprgen.h"
#include "codegen/dag_visualizer.h"

int main(int, char**){
    rain::Lexer lexer(rain::readall("./text.txt"));

    lexer.lex_all();
    lexer.token_sequence.push_back(new rain::Token(rain::TokenType::ENDMARK, "$", rain::makepos(lexer.pos)));
    std::cout << "Tokens:" << std::endl;

    for (const auto &tok : lexer.token_sequence) {
        std::cout << tok->repr() << std::endl;
    }

    auto res = rain::AddExprNode::parse(lexer.token_sequence.begin(), lexer.token_sequence.end());
    auto *root_expr = res.val;

    if (res.success) {
        std::cout << "Parsed successfully!" << std::endl;
        std::cout << res.end - lexer.token_sequence.begin() << std::endl;
        rain::generate_ast_dot_to_file("ast.dot", root_expr);
    } else {
        std::cout << "Parse failed!" << std::endl;
    }

    rain::CompDAG dag;
    rain::SymbolTable symtab;
    rain::CodeGenContext ctx(symtab);
    
    rain::CompNode *root = rain::general_expr_gen(root_expr, dag, ctx);
    rain::DAGVisualizer::export_to_dot(dag, "dag.dot", ctx);

    rain::IASTNode::pool.cleanup();
    rain::Token::pool.cleanup();
    rain::PosInfo::pool.cleanup();

    return 0;
}
