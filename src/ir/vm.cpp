#include "vm.h"

#include <iostream>

void rain::ExecEnv::exec_instruction(const Instruction &instr)
{
    switch (instr.opcode) {
    case Instruction::OpCode::LOAD_CONST:
        push_val(instr.operand);
        break;
    case Instruction::OpCode::STORE_VAR:
        set_var(instr.operand, pop_val());
        break;
    case Instruction::OpCode::LOAD_VAR:
        push_val(get_var(instr.operand));
        break;
    case Instruction::OpCode::INC: {
        int val = pop_val();
        push_val(val + 1);
        break;
    }
    case Instruction::OpCode::RETURN: {
        // 从栈顶取出值压入到返回过程的栈中
        int ret_val = pop_val();
        framestack.pop();
        if (!framestack.empty()) {
            push_val(ret_val);
        }
        break;
    }
    case Instruction::OpCode::STORE_PARAM: {
        // Handle store param
        framestack.top().caller_param_table[instr.operand] = pop_val();
        break;
    }
    case Instruction::OpCode::LOAD_PARAM: {
        // Handle load param
        auto it = framestack.top().callee_param_table.find(instr.operand);
        if (it != framestack.top().callee_param_table.end()) {
            push_val(it->second);
        } else {
            std::cout << "Parameter not found: " << instr.operand << std::endl;
            push_val(0);
        }
        break;
    }
    case Instruction::OpCode::CALL: {
        // 新建一个帧并将其压入栈中
        const Process *proc = pro.get_nth_process(instr.operand); // For simplicity, always call main process
        if (proc == nullptr) {
            std::cout << "Invalid process index: " << instr.operand << std::endl;
            break;
        }   
        Frame &frame = framestack.top();
        framestack.push(Frame{proc, std::stack<int>(), {}, frame.caller_param_table, -1});
        break;
    }
    case Instruction::OpCode::PRINT: {
        int val = pop_val();
        std::cout << "PRINT: " << val << std::endl;
        break;
    }
    case Instruction::OpCode::HALT:
        halt = true;
        break;
    default:
        break;
    }
}

void rain::ExecEnv::run()
{
    while (framestack.top().pc < framestack.top().proc->get_instructions().size()) {
        exec_instruction(framestack.top().proc->get_instructions()[framestack.top().pc]);
        framestack.top().pc++;
        if (halt) {
            break;
        }
    }
}
