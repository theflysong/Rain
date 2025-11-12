#pragma once

#include "codegen/symtab.h"
#include "parser/ast.h"
#include "ir/vm.h"

#include <unordered_map>
#include <vector>
#include <string>

namespace rain {
    struct CodeGenContext {
        SymbolTable &symtab;

        // VM emission state
        std::vector<rainvm::Instruction> main_instructions; // main buffer
        std::vector<std::vector<rainvm::Instruction>*> instr_buf_stack; // current buffer stack
        std::unordered_map<std::string, int> var_slot; // top-level variable -> slot id
        int next_var_id{0};

        // Process list (index 0 reserved for main during finalize)
        std::vector<rainvm::Process*> processes;

        // Parameter scopes: each is name -> param index (0-based)
        std::vector<std::unordered_map<std::string,int>> param_scopes;

        // Capture environment stacks for current abstraction being compiled
        std::vector<std::unordered_map<std::string,int>> capture_index_stack; // name -> captured index
        std::vector<std::vector<std::string>> capture_order_stack;            // preserve order

        CodeGenContext(SymbolTable &symtab)
            : symtab(symtab)
        {
        }

        int ensure_var_slot(const std::string &name) {
            auto it = var_slot.find(name);
            if (it != var_slot.end()) return it->second;
            int id = next_var_id++;
            var_slot[name] = id;
            return id;
        }

        // Current instruction buffer ref
        std::vector<rainvm::Instruction>& cur_buf() {
            if (instr_buf_stack.empty()) return main_instructions;
            return *instr_buf_stack.back();
        }

        // Capture helpers
        bool has_active_capture() const { 
            return !capture_index_stack.empty(); 
        }
        int ensure_capture_index(const std::string &name) {
            if (capture_index_stack.empty()) return -1;
            auto &m = capture_index_stack.back();
            auto it = m.find(name);
            if (it != m.end()) return it->second;
            int idx = static_cast<int>(capture_order_stack.back().size());
            m[name] = idx;
            capture_order_stack.back().push_back(name);
            return idx;
        }

        // Build a Program containing main + additional processes
        rainvm::Program *finalize_program() {
            auto *main_proc = new rainvm::Process(std::move(main_instructions), 0);
            std::vector<rainvm::Process *> procs;
            procs.reserve(processes.size() + 1);
            procs.push_back(main_proc);
            procs.insert(procs.end(), processes.begin(), processes.end());
            return new rainvm::Program(std::move(procs), main_proc);
        }
    };

    // type checking
    Type *gen_type(const ITypeAST *type_node, CodeGenContext &ctx);
    Type *gen_literal_expr(const ILiteralExprAST *literal_node, CodeGenContext &ctx); 
    Type *gen_successor_expr(const ISuccExprAST *succ_node, CodeGenContext &ctx);
    Type *gen_abstraction_expr(const IAbstractionAST *abst_node, CodeGenContext &ctx);
    Type *gen_identifier_expr(const IIdentifierExprAST *ident_node, CodeGenContext &ctx);
    Type *gen_application_expr(const IApplicationAST *app_node, CodeGenContext &ctx);
    Type *gen_expr(const IExprAST *expr_node, CodeGenContext &ctx);

    bool gen_stmt(const IStmtAST *stmt_node, CodeGenContext &ctx);
    bool gen_program(const IProgramAST *program_node, CodeGenContext &ctx);
}