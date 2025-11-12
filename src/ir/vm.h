#pragma once

#include <unordered_map>
#include <stack>
#include <vector>

namespace rain {
    struct Instruction {
        enum class OpCode {
            NOP,
            LOAD_CONST,
            STORE_VAR,
            LOAD_VAR,
            INC,
            RETURN,
            STORE_PARAM,
            LOAD_PARAM,
            CALL,
            PRINT,
            HALT
        } opcode;
        int operand;
    };

    class Process {
    protected:
        std::vector<Instruction> instructions;
        int param_count;
    public:
        Process(std::vector<Instruction>&& instrs, int param_cnt)
            : instructions(std::move(instrs)), param_count(param_cnt)
        {
        }

        const std::vector<Instruction>& get_instructions() const {
            return instructions;
        }

        int get_param_count() const {
            return param_count;
        }
    };

    class Program {
    protected:
        std::vector<Process *> processes;
        Process *main_process;
    public:
        Program(std::vector<Process *>&& procs, Process *main_proc)
            : processes(std::move(procs)), main_process(main_proc)
        {
        }

        ~Program() {
            for (auto proc : processes) {
                delete proc;
            }
        }

        const Process *get_main_process() const {
            return main_process;
        }

        const Process *get_nth_process(int n) const {
            if (n < 0 || n >= processes.size()) {
                return nullptr;
            }
            return processes[n];
        }
    };

    class ExecEnv {
    protected:
        struct Frame {
            const Process *proc;
            std::stack<int> valstack;
            std::unordered_map<int, int> caller_param_table;
            std::unordered_map<int, int> callee_param_table;
            int pc;
        };
        std::unordered_map<int, int> variables;
        const Program &pro;
        std::stack<Frame> framestack;
        bool halt;
    public:
        ExecEnv(const Program &program) : variables(), pro(program), framestack(), halt(false) {
            framestack.push(Frame{pro.get_main_process(), std::stack<int>(), {}, {}, 0});
        }

        void set_var(int var_id, int value) {
            variables[var_id] = value;
        }

        int get_var(int var_id) const {
            auto it = variables.find(var_id);
            if (it != variables.end()) {
                return it->second;
            }
            return 0;
        }

        void push_val(int value) {
            framestack.top().valstack.push(value);
        }

        int pop_val() {
            if (framestack.top().valstack.empty()) {
                return 0;
            }
            int val = framestack.top().valstack.top();
            framestack.top().valstack.pop();
            return val;
        }

        void exec_instruction(const Instruction &instr);
        void run();
    };
}