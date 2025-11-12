#include "codegen.h"
#include <iostream>

namespace rain {
    // Minimal emit helper kept for inlined emission
    namespace {
        inline void emit(std::vector<rainvm::Instruction> &buf, rainvm::Instruction::OpCode op, int operand = 0) {
            std::cout << "Emitting instruction: OpCode=" << op_to_cstr(op) << ", Operand=" << operand << std::endl;
            buf.push_back(rainvm::Instruction{op, operand});
        }

        struct ParamLookup { bool found{false}; int index{-1}; int depth{-1}; };
        static ParamLookup lookup_param(const CodeGenContext &ctx, const std::string &name) {
            for (int d = static_cast<int>(ctx.param_scopes.size()) - 1; d >= 0; --d) {
                auto it = ctx.param_scopes[d].find(name);
                if (it != ctx.param_scopes[d].end()) {
                    return {true, it->second, d};
                }
            }
            return {};
        }
    }

    Type *gen_type(const ITypeAST *type_node, CodeGenContext &ctx){
        auto terms = type_node->flatten();
        if (terms.empty()) {
            return nullptr;
        }
        
        Type *result_type = nullptr;
        int last_idx = terms.size() - 1;

        if (terms[last_idx].is_nat) {
            result_type = &Type::NAT_TYPE;
        } else {
            result_type = gen_type(terms[last_idx].subtype, ctx);
        }

        // 从尾解析到头
        for (int i = last_idx - 1; i >= 0; i--) {
            Type *arg_type = nullptr;
            if (terms[i].is_nat) {
                arg_type = &Type::NAT_TYPE;
            } 
            else if (terms[i].subtype != nullptr) {
                arg_type = gen_type(terms[i].subtype, ctx);
            }
            
            if (arg_type == nullptr) {
                return nullptr;
            }
            result_type = new Type(arg_type, result_type);
        }

        return result_type;
    }

    Type *gen_literal_expr(const ILiteralExprAST *literal_node, CodeGenContext &ctx)
    {
        // Emit runtime: push constant
        const Token *tok = literal_node->get_literal_token();

        long long val = 0;
        try { 
            val = std::stoll(tok->lexeme, nullptr, 0); 
        } catch (...) {
            val = 0;
        }
        // emit to current buffer (gen_program will set it)
        emit(*ctx.instr_buf_stack.back(), rainvm::Instruction::OpCode::LOAD_CONST, static_cast<int>(val));

        return &Type::NAT_TYPE;
    }

    Type *gen_successor_expr(const ISuccExprAST *succ_node, CodeGenContext &ctx) {
        if (succ_node->num_succ() <= 0) {
            return gen_expr(succ_node->sub_expr(), ctx);
        }

        // First evaluate subexpr (also emits)
        Type *expr_type = gen_expr(succ_node->sub_expr(), ctx);

        // 判断子式类型 : 必须为nat
        if (expr_type == nullptr || type_match(expr_type, &Type::NAT_TYPE) == false) {
            std::cout << "Successor expression applied to non-Nat type." << std::endl;
            return nullptr;
        }

        if (succ_node->num_succ() <= 0) {
            return &Type::NAT_TYPE;
        }

        // Emit INC n times
        for (int i = 0; i < succ_node->num_succ(); ++i) {
            emit(*ctx.instr_buf_stack.back(), rainvm::Instruction::OpCode::INC, 0);
        }

        return &Type::NAT_TYPE;
    }

    Type *gen_abstraction_expr(const IAbstractionAST *abst_node, CodeGenContext &ctx)
    {
        Type *param_type = gen_type(abst_node->parameter_type(), ctx);
        if (param_type == nullptr) {
            std::cout << "Failed to generate type for abstraction parameter." << std::endl;
            return nullptr;
        }

        std::string param_name = abst_node->parameter_name();

        // 进入类型环境（参数在类型检查中可见）
        ctx.symtab.enter_level();
        ctx.symtab.lookup_or_insert_variable(param_name, param_type);

        // 第一次：仅类型检查，避免向主缓冲发射
        std::vector<rainvm::Instruction> dummy;
        ctx.instr_buf_stack.push_back(&dummy);
        Type *body_type = gen_expr(abst_node->body_expr(), ctx);
        ctx.instr_buf_stack.pop_back();
        if (body_type == nullptr) {
            std::cout << "Failed to generate type for abstraction body." << std::endl;
            return nullptr;
        }

        // 第二次：生成过程体代码到独立缓冲
        std::vector<rainvm::Instruction> body_buf;
        ctx.param_scopes.push_back({{param_name, 0}});
        ctx.capture_index_stack.push_back({});
        ctx.capture_order_stack.push_back({});
        ctx.instr_buf_stack.push_back(&body_buf);
        Type *body_type_emit = gen_expr(abst_node->body_expr(), ctx);
        emit(body_buf, rainvm::Instruction::OpCode::RETURN, 0);

        // 捕获列表
        auto captured_names = ctx.capture_order_stack.back();
        ctx.instr_buf_stack.pop_back();
        ctx.capture_order_stack.pop_back();
        ctx.capture_index_stack.pop_back();
        ctx.param_scopes.pop_back();

        // 退出类型环境
        ctx.symtab.exit_level();

        if (body_type_emit && body_type_emit != body_type) {
            delete body_type_emit;
        }

        // 建立子进程并登记
        auto *proc = new rainvm::Process(std::move(body_buf), /*param_cnt*/ 1);
        int proc_index = static_cast<int>(ctx.processes.size()) + 1; // 0 留给 main
        ctx.processes.push_back(proc);

        // 在当前缓冲（外层）生成闭包：依次压入捕获值，压入捕获数量，然后 MAKE_CLOSURE
        auto &outer_buf = *ctx.instr_buf_stack.back();
        for (const auto &nm : captured_names) {
            auto lk2 = lookup_param(ctx, nm);
            if (lk2.found) {
                if (lk2.depth == static_cast<int>(ctx.param_scopes.size()) - 1) {
                    emit(outer_buf, rainvm::Instruction::OpCode::LOAD_PARAM, lk2.index);
                } else {
                    int cap_idx2 = ctx.ensure_capture_index(nm);
                    if (cap_idx2 < 0) cap_idx2 = 0;
                    emit(outer_buf, rainvm::Instruction::OpCode::LOAD_CAPTURED_VAR, cap_idx2);
                }
            } else {
                int slot = ctx.ensure_var_slot(nm);
                emit(outer_buf, rainvm::Instruction::OpCode::LOAD_VAR, slot);
            }
        }
        emit(outer_buf, rainvm::Instruction::OpCode::LOAD_CONST, static_cast<int>(captured_names.size()));
        emit(outer_buf, rainvm::Instruction::OpCode::MAKE_CLOSURE, proc_index);
        std::cout << "[codegen] build closure proc#" << proc_index << " captures=";
        for (size_t i = 0; i < captured_names.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << captured_names[i];
        }
        std::cout << std::endl;

        return new Type(param_type, body_type);
    }

    Type *gen_identifier_expr(const IIdentifierExprAST *ident_node, CodeGenContext &ctx)
    {
        auto sym = ctx.symtab.lookup(ident_node->get_identifier());
        if (sym == nullptr) {
            std::cout << "Undefined identifier name: " << ident_node->get_identifier() << std::endl;
        }
        if (sym->type != SymbolTypes::SYMBOL_IDENTIFIER) {
            std::cout << "Identifier " << ident_node->get_identifier() << " is not a identifier for variable" << std::endl;
        }
    // variable type now stored directly in SymbolEntry
    Type *var_ty = sym->var_type;
        const std::string name = ident_node->get_identifier();
        // 解析为参数/捕获/全局
        auto lk = lookup_param(ctx, name);
        if (lk.found) {
            if (lk.depth == static_cast<int>(ctx.param_scopes.size()) - 1) {
                emit(*ctx.instr_buf_stack.back(), rainvm::Instruction::OpCode::LOAD_PARAM, lk.index);
            } else {
                int cap_idx = ctx.ensure_capture_index(name);
                if (cap_idx < 0) cap_idx = 0;
                emit(*ctx.instr_buf_stack.back(), rainvm::Instruction::OpCode::LOAD_CAPTURED_VAR, cap_idx);
            }
        } else {
            int slot = ctx.ensure_var_slot(name);
            emit(*ctx.instr_buf_stack.back(), rainvm::Instruction::OpCode::LOAD_VAR, slot);
        }
    return var_ty;
    }

    Type *gen_application_expr(const IApplicationAST *app_node, CodeGenContext &ctx)
    {
        // 生成函数与参数，并在每个参数后 APPLY（柯里化）
        Type *func_type = gen_expr(app_node->function_expr(), ctx);
        for (auto *arg_ast : app_node->argument_expr()) {
            Type *arg_type = gen_expr(arg_ast, ctx);
            if (arg_type == nullptr) {
                std::cout << "Cannot parse type for argument" << std::endl;
                return nullptr;
            }

            Type *ret_type = func_type->apply(arg_type);
            if (ret_type == nullptr) {
                std::cout << "Function type " << func_type->repr() << " cannot be applied to argument type " << arg_type->repr() << std::endl;
                return nullptr;
            }
            // 运行时：将参数作为 param#0 传入并应用
            emit(*ctx.instr_buf_stack.back(), rainvm::Instruction::OpCode::STORE_PARAM, 0);
            emit(*ctx.instr_buf_stack.back(), rainvm::Instruction::OpCode::APPLY, 0);
            std::cout << "[codegen] APPLY one argument" << std::endl;

            func_type = ret_type;
        }
        return func_type;
    }

    Type *gen_expr(const IExprAST *expr_node, CodeGenContext &ctx)
    {
        switch (expr_node->get_expr_type()) {
            case ExpressionTypes::LITERAL: {
                const ILiteralExprAST *lit_node = dynamic_cast<const ILiteralExprAST *>(expr_node);
                if (lit_node == nullptr) {
                    return nullptr;
                }
                return gen_literal_expr(lit_node, ctx);
            }
            case ExpressionTypes::ABSTRACTION: {
                const IAbstractionAST *abst_node = dynamic_cast<const IAbstractionAST *>(expr_node);
                if (abst_node == nullptr) {
                    return nullptr;
                }
                return gen_abstraction_expr(abst_node, ctx);
            }
            case ExpressionTypes::SUCCESSOR: {
                const ISuccExprAST *succ_node = dynamic_cast<const ISuccExprAST *>(expr_node);
                if (succ_node == nullptr) {
                    return nullptr;
                }
                return gen_successor_expr(succ_node, ctx);
            }
            case ExpressionTypes::GENERAL_EXPR: {
                // 直接返回内部表达式的类型
                const IGeneralExprAST *gen_node = dynamic_cast<const IGeneralExprAST *>(expr_node);
                if (gen_node == nullptr) {
                    return nullptr;
                }
                return gen_expr(gen_node->inner_expr(), ctx);
            }
            case ExpressionTypes::PRIM_EXPR: {
                // 直接返回内部表达式的类型
                const IPrimExprAST *prim_node = dynamic_cast<const IPrimExprAST *>(expr_node);
                if (prim_node == nullptr) {
                    return nullptr;
                }
                return gen_expr(prim_node->inner_expr(), ctx);
            }
            case ExpressionTypes::IDENTIFIER: {
                const IIdentifierExprAST *ident_node = dynamic_cast<const IIdentifierExprAST *>(expr_node);
                if (ident_node == nullptr) {
                    return nullptr;
                }
                return gen_identifier_expr(ident_node, ctx);
            }
            case ExpressionTypes::APPLICATION: {
                const IApplicationAST *app_node = dynamic_cast<const IApplicationAST *>(expr_node);
                if (app_node == nullptr) {
                    return nullptr;
                }
                return gen_application_expr(app_node, ctx);
            }
            default:
                return nullptr;
        }
        return nullptr;
    }

    bool gen_let_stmt(const ILetStmtAST *let_node, CodeGenContext &ctx)
    {
        std::string identifier = let_node->identifier();
        ITypeAST *type_decl = let_node->type_decl();
        IExprAST *value_expr = let_node->value_expr();

        Type *var_type = gen_type(type_decl, ctx);
        if (var_type == nullptr) {
            std::cout << "Failed to parse type declaration for let statement." << std::endl;
            return false;
        }

        std::cout << "Generated type for variable '" << identifier << "': " << var_type->repr() << std::endl;

        Type *expr_type = gen_expr(value_expr, ctx);
        if (expr_type == nullptr) {
            std::cout << "Failed to parse type for value expression in let statement." << std::endl;
            return false;
        }

        std::cout << "Generated type for value expression: " << expr_type->repr() << std::endl;

        if (!type_match(var_type, expr_type)) {
            std::cout << "Type mismatch in let statement for variable '" << identifier << "'." << std::endl;
            return false;
        }

        std::cout << "Type match confirmed for variable '" << identifier << "'." << std::endl;
        ctx.symtab.lookup_or_insert_variable(identifier, var_type);

        // 存储变量
        int slot = ctx.ensure_var_slot(identifier);
        emit(*ctx.instr_buf_stack.back(), rainvm::Instruction::OpCode::STORE_VAR, slot);

        return true;
    }

    bool gen_print_stmt(const IPrintStmtAST *print_node, CodeGenContext &ctx)
    {
        IExprAST *expr = print_node->expr();
        Type *expr_type = gen_expr(expr, ctx);
        if (expr_type == nullptr) {
            std::cout << "Failed to generate type for print expression." << std::endl;
            return false;
        }

        if (type_match(expr_type, &Type::NAT_TYPE) == false) {
            std::cout << "Print expression must be of type Nat, but got: " << expr_type->repr() << std::endl;
            return false;
        }

        std::cout << "Generated type for print expression: " << expr_type->repr() << std::endl;
        // 输出变量
        emit(*ctx.instr_buf_stack.back(), rainvm::Instruction::OpCode::PRINT, 0);
    

        return true;
    }

    bool gen_stmt(const IStmtAST *stmt_node, CodeGenContext &ctx)
    {
        if (const IPrintStmtAST *print_stmt = dynamic_cast<const IPrintStmtAST *>(stmt_node)) {
            return gen_print_stmt(print_stmt, ctx);
        }
        else if (const ILetStmtAST *let_stmt = dynamic_cast<const ILetStmtAST *>(stmt_node)) {
            return gen_let_stmt(let_stmt, ctx);
        }
        return false;
    }

    bool gen_program(const IProgramAST *program_node, CodeGenContext &ctx)
    {
        ctx.main_instructions.clear();
        ctx.instr_buf_stack.clear();
        ctx.processes.clear();
        // 将主缓冲压栈
        ctx.instr_buf_stack.push_back(&ctx.main_instructions);
        for (const IStmtAST *stmt : program_node->statements()) {
            if (! gen_stmt(stmt, ctx)) {
                return false;
            }
        }
        emit(*ctx.instr_buf_stack.back(), rainvm::Instruction::OpCode::HALT, 0);
        ctx.instr_buf_stack.pop_back();
        return true;
    }
}