#include <iostream>

#include "lexer/lexer.h"
#include "parser/syntax.h"
#include "file/helper.h"
#include "parser/syntax_dot.h"
#include "codegen/codegen.h"
#include "ir/vm.h"

static void dump_program(const rainvm::Program &program) {
    std::cout << "===== VM Program Dump =====" << std::endl;
    int n = program.process_count();
    for (int i = 0; i < n; ++i) {
        auto *proc = program.get_nth_process(i);
        if (!proc) continue;
        std::cout << "Process #" << i << " (params=" << proc->get_param_count() << ")" << std::endl;
        const auto &ins = proc->get_instructions();
        for (size_t pc = 0; pc < ins.size(); ++pc) {
            std::cout << "  [" << pc << "] " << op_to_cstr(ins[pc].opcode) << " " << ins[pc].operand << std::endl;
        }
    }
    std::cout << "===========================" << std::endl;
}

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
        // finalize and run VM program
        std::unique_ptr<rainvm::Program> prog(ctx.finalize_program());
        dump_program(*prog);
        rainvm::ExecEnv env(*prog);
        env.run();
    } else {
        std::cout << "Program generation failed!" << std::endl;
    }
}

void testbench_2() {
    using namespace rainvm;

    std::vector<Process *> procs;
    Process *proc1 = new Process({
        {Instruction::OpCode::LOAD_PARAM, 0},
        {Instruction::OpCode::INC},
        {Instruction::OpCode::INC},
        {Instruction::OpCode::RETURN, 0}
    }, 0);
    Process *main_proc = new Process({
        {Instruction::OpCode::LOAD_CONST, 63},
        {Instruction::OpCode::STORE_PARAM, 0},
        {Instruction::OpCode::CALL, 1},
        {Instruction::OpCode::PRINT, 0},
        {Instruction::OpCode::HALT, 0}
    }, 0);
    Program vm_procs({main_proc, proc1}, main_proc);
    ExecEnv env(vm_procs);
    env.run();
}

// 使用闭包功能的简单测试：
// 1) 在 main 中构造一个捕获值为 10 的闭包，随后将其捕获改为 20
// 2) 调用该闭包，闭包体仅返回其捕获值
// 期望输出：PRINT: 20
void testbench_closure() {
    using namespace rainvm;

    // 闭包体：读取捕获值 #0 并返回
    Process *closure_body = new Process({
        {Instruction::OpCode::LOAD_CAPTURED_VAR, 0},
        {Instruction::OpCode::INC},
        {Instruction::OpCode::RETURN, 0},
    }, /*param_cnt*/ 0);

    // 主过程：
    // 栈: [10] [1] -> MAKE_CLOSURE( index_of(closure_body) ) -> [closure]
    // 然后将 20 写入闭包捕获 #0: push 20; STORE_CAPTURED_VAR 0
    // 应用闭包 -> 返回捕获值
    // 打印并停止
    Process *main_proc = new Process({
        {Instruction::OpCode::LOAD_CONST, 10},     // 捕获值0
        {Instruction::OpCode::LOAD_CONST, 1},      // 捕获数量
        {Instruction::OpCode::MAKE_CLOSURE, 1},    // processes[1] = closure_body

        {Instruction::OpCode::LOAD_CONST, 20},     // 新值
        {Instruction::OpCode::STORE_CAPTURED_VAR, 0},

        {Instruction::OpCode::APPLY, 0},           // 应用栈顶闭包
        {Instruction::OpCode::PRINT, 0},
        {Instruction::OpCode::HALT, 0},
    }, /*param_cnt*/ 0);

    Program program({main_proc, closure_body}, main_proc);
    ExecEnv env(program);
    env.run();
}

int main(int, char**){
    testbench_1();
    // testbench_2();
    // testbench_closure();

    rain::IASTNode::pool.cleanup();
    rain::Token::pool.cleanup();
    rain::PosInfo::pool.cleanup();

    return 0;
}
