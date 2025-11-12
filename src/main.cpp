#include <iostream>

#include "lexer/lexer.h"
#include "parser/syntax.h"
#include "file/helper.h"
#include "parser/syntax_dot.h"
#include "codegen/codegen.h"
#include "ir/vm.h"

void testbench_1() {
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
}

void testbench_2() {

    std::vector<rain::Process *> procs;
    rain::Process *proc1 = new rain::Process({
        {rain::Instruction::OpCode::LOAD_PARAM, 0},
        {rain::Instruction::OpCode::INC},
        {rain::Instruction::OpCode::INC},
        {rain::Instruction::OpCode::RETURN, 0}
    }, 0);
    rain::Process *main_proc = new rain::Process({
        {rain::Instruction::OpCode::LOAD_CONST, 63},
        {rain::Instruction::OpCode::STORE_PARAM, 0},
        {rain::Instruction::OpCode::CALL, 1},
        {rain::Instruction::OpCode::PRINT, 0},
        {rain::Instruction::OpCode::HALT, 0}
    }, 0);
    rain::Program vm_procs({main_proc, proc1}, main_proc);
    rain::ExecEnv env(vm_procs);
    env.run();
}

int main(int, char**){
    // testbench_1();
    testbench_1();

    rain::IASTNode::pool.cleanup();
    rain::Token::pool.cleanup();
    rain::PosInfo::pool.cleanup();

    return 0;
}
