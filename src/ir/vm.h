#pragma once

#include <unordered_map>
#include <stack>
#include <vector>
#include <memory>

namespace rainvm {
    struct Instruction {
        enum class OpCode {
            NOP,
            LOAD_CONST, // 加载第i个变量(可能为闭包)
            STORE_VAR,  // 存储第i个变量(可能为闭包)
            LOAD_VAR,
            INC,
            RETURN,
            STORE_PARAM,
            LOAD_PARAM,
            CALL,
            PRINT,
            HALT,
            LOAD_CAPTURED_VAR,  // 从栈顶获取闭包, 加载第operand个捕获值
            STORE_CAPTURED_VAR, // 从栈顶-1获取闭包, 将栈顶存储为第operand个捕获值
            MAKE_CLOSURE,       // 构造一个指向第 operand 个 process 的闭包
            APPLY,              // 应用栈顶闭包
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

    struct Closure; // 前向声明，Value 中以指针形式引用

    struct Value {
        enum class Kind { Int, Closure } kind;
        int nat_value{}; // 当 kind == Int 时有效
        std::shared_ptr<Closure> clos; // 当 kind == Closure 时有效

        Value() : kind(Kind::Int), nat_value(0), clos(nullptr) {}
        explicit Value(int v) : kind(Kind::Int), nat_value(v), clos(nullptr) {}
        explicit Value(std::shared_ptr<Closure> c) : kind(Kind::Closure), nat_value(0), clos(std::move(c)) {}

        bool is_int() const { return kind == Kind::Int; }
        bool is_closure() const { return kind == Kind::Closure; }

        int as_int() const { return is_int() ? nat_value : 0; }
        std::shared_ptr<Closure> as_closure() const { return is_closure() ? clos : nullptr; }
    };

    struct Closure {
        const Process *proc;
        std::vector<Value> captured; // 按索引访问捕获值

        Closure(const Process *p, std::vector<Value> cap)
            : proc(p), captured(std::move(cap)) {}
    };

    class ExecEnv {
    protected:
        struct Frame {
            const Process *proc;
            std::stack<Value> valstack;
            std::unordered_map<int, Value> variables;
            std::unordered_map<int, Value> caller_param_table;
            std::unordered_map<int, Value> callee_param_table;
            int pc;
        };
        const Program &pro;
        std::stack<Frame> framestack;
        bool halt;
    public:
        ExecEnv(const Program &program) : pro(program), framestack(), halt(false) {
            framestack.push(Frame{pro.get_main_process(), std::stack<Value>(), {}, {}, {}, 0});
        }

        const Frame& frame() const{
            return framestack.top();
        }

        Frame& frame() {
            return framestack.top();
        }

        void set_var(int var_id, Value value) {
            frame().variables[var_id] = value;
        }

        Value get_var(int var_id) const {
            auto it = frame().variables.find(var_id);
            if (it != frame().variables.end()) {
                return it->second;
            }
            return Value{0};
        }

        void push_val(Value value) {
            framestack.top().valstack.push(value);
        }

        Value pop_val() {
            if (framestack.top().valstack.empty()) {
                return Value{0};
            }
            Value val = framestack.top().valstack.top();
            framestack.top().valstack.pop();
            return val;
        }

        const Value& peek_val() const {
            static Value zero{};
            if (framestack.top().valstack.empty()) return zero;
            return framestack.top().valstack.top();
        }

        void exec_instruction(const Instruction &instr);
        void run();
    };
}