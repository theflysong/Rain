#include "vm.h"

#include <iostream>

void rainvm::ExecEnv::exec_instruction(const Instruction &instr)
{
    switch (instr.opcode) {
    case Instruction::OpCode::LOAD_CONST:
        push_val(Value(instr.operand));
        break;
    case Instruction::OpCode::STORE_VAR:
        set_var(instr.operand, pop_val());
        break;
    case Instruction::OpCode::LOAD_VAR:
        push_val(get_var(instr.operand));
        break;
    case Instruction::OpCode::INC: {
        int val = pop_val().as_int();
        push_val(Value(val + 1));
        break;
    }
    case Instruction::OpCode::RETURN: {
        // 从栈顶取出值压入到返回过程的栈中
        auto ret_val = pop_val();
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
            push_val(Value(0));
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
        Frame &caller = framestack.top();
        // 将实参从 caller.caller_param_table 传递给被调帧的 callee_param_table
        auto callee_params = caller.caller_param_table; // 拷贝
        framestack.push(Frame{proc, std::stack<Value>(), {}, {}, callee_params, -1});
        // 可选：清空调用者的参数表，避免泄漏到后续调用
        caller.caller_param_table.clear();
        break;
    }
    case Instruction::OpCode::PRINT: {
        Value v = pop_val();
        if (v.is_int()) {
            std::cout << "PRINT: " << v.as_int() << std::endl;
        } else {
            std::cout << "PRINT: [closure]" << std::endl;
        }
        break;
    }
    case Instruction::OpCode::HALT:
        halt = true;
        break;
    case Instruction::OpCode::LOAD_CAPTURED_VAR: {
        // 从栈顶获取闭包，不弹出
        const Value &top = peek_val();
        if (!top.is_closure()) {
            std::cout << "LOAD_CAPTURED_VAR expects closure on stack top" << std::endl;
            push_val(Value(0));
            break;
        }
        auto clos = top.as_closure();
        int idx = instr.operand;
        if (!clos || idx < 0 || idx >= static_cast<int>(clos->captured.size())) {
            std::cout << "Captured index out of range: " << idx << std::endl;
            push_val(Value(0));
            break;
        }
        push_val(clos->captured[idx]);
        break;
    }
    case Instruction::OpCode::STORE_CAPTURED_VAR: {
        // 从栈顶-1 获取闭包，将栈顶存储为第 operand 个捕获值
        Value val = pop_val();
        if (framestack.top().valstack.empty() || !framestack.top().valstack.top().is_closure()) {
            std::cout << "STORE_CAPTURED_VAR expects closure under top" << std::endl;
            break;
        }
        auto clos = framestack.top().valstack.top().as_closure();
        int idx = instr.operand;
        if (!clos) break;
        if (idx < 0) break;
        if (idx >= static_cast<int>(clos->captured.size())) {
            // 自动扩容到需要的大小
            clos->captured.resize(idx + 1, Value(0));
        }
        clos->captured[idx] = val;
        break;
    }
    case Instruction::OpCode::MAKE_CLOSURE: {
        // 约定：栈顶为捕获数量 N，之下紧接着 N 个捕获值（最先压入的捕获值在最底）
        int count = pop_val().as_int();
        if (count < 0) count = 0;
        std::vector<Value> caps(count);
        // 逆序弹出以保持 captured[0] 对应最早压入的值
        for (int i = count - 1; i >= 0; --i) {
            caps[i] = pop_val();
        }
        const Process *proc = pro.get_nth_process(instr.operand);
        if (!proc) {
            std::cout << "MAKE_CLOSURE invalid process index: " << instr.operand << std::endl;
            push_val(Value(0));
            break;
        }
        auto clos = std::make_shared<Closure>(proc, std::move(caps));
        push_val(Value(clos));
        break;
    }
    case Instruction::OpCode::APPLY: {
        // 应用栈顶闭包：弹出闭包，创建新帧，放入闭包到新帧的栈顶，传递参数表
        Value v = pop_val();
        if (!v.is_closure()) {
            std::cout << "APPLY expects a closure on stack" << std::endl;
            break;
        }
        auto clos = v.as_closure();
        if (!clos || !clos->proc) {
            std::cout << "APPLY got invalid closure" << std::endl;
            break;
        }
        Frame &caller = framestack.top();
        auto callee_params = caller.caller_param_table; // 将调用者准备好的参数作为被调者的 callee 参数
        // 新帧并将闭包本身压入其栈，方便 LOAD_CAPTURED_VAR 使用
        Frame new_frame{clos->proc, std::stack<Value>(), {}, {}, callee_params, -1};
        new_frame.valstack.push(Value(clos));
        framestack.push(std::move(new_frame));
        caller.caller_param_table.clear();
        break;
    }
    default:
        break;
    }
}

void rainvm::ExecEnv::run()
{
    while (framestack.top().pc < framestack.top().proc->get_instructions().size()) {
        exec_instruction(framestack.top().proc->get_instructions()[framestack.top().pc]);
        framestack.top().pc++;
        if (halt) {
            break;
        }
    }
}
