#include "codegen.h"

namespace rain {
    Type *gen_type(const ITypeAST *type_node, CodeGenContext &ctx){
        auto terms = type_node->flatten();
        if (terms.empty()) {
            return nullptr;
        }
        
        Type *result_type = nullptr;
        int last_idx = terms.size() - 1;

        if (terms[last_idx].is_nat) {
            result_type = new Type(TypeKinds::TYPE_NAT);
        } else {
            result_type = gen_type(terms[last_idx].subtype, ctx);
        }

        // 从尾解析到头
        for (int i = last_idx - 1; i >= 0; i--) {
            Type *arg_type = nullptr;
            if (terms[i].is_nat) {
                arg_type = new Type(TypeKinds::TYPE_NAT);
            } 
            else if (terms[i].subtype != nullptr) {
                arg_type = gen_type(terms[i].subtype, ctx);
            }
            
            if (arg_type == nullptr) {
                delete result_type;
                return nullptr;
            }
            result_type = new Type(arg_type, result_type);
        }

        return result_type;
    }

    Type *gen_literal_expr_type(const ILiteralExprAST *literal_node, CodeGenContext &ctx)
    {
        return new Type(TypeKinds::TYPE_NAT);
    }

    Type *gen_successor_expr_type(const ISuccExprAST *succ_node, CodeGenContext &ctx) {
        if (succ_node->num_succ() <= 0) {
            return gen_expr_type(succ_node->sub_expr(), ctx);
        }
        
        Type *expr_type = gen_expr_type(succ_node->sub_expr(), ctx);

        if (expr_type->kind != TypeKinds::TYPE_NAT) {
            std::cout << "Successor expression applied to non-Nat type." << std::endl;
            delete expr_type;
            return nullptr;
        }

        delete expr_type;
        return new Type(TypeKinds::TYPE_NAT);
    }

    Type *gen_abstraction_expr_type(const IAbstractionAST *abst_node, CodeGenContext &ctx)
    {
        Type *param_type = gen_type(abst_node->parameter_type(), ctx);
        if (param_type == nullptr) {
            std::cout << "Failed to generate type for abstraction parameter." << std::endl;
            return nullptr;
        }

        std::string param_name = abst_node->parameter_name();

        ctx.symtab.enter_level();
        // 在符号表中插入参数类型信息
        ctx.symtab.lookup_or_insert_variable(param_name, param_type);

        Type *body_type = gen_expr_type(abst_node->body_expr(), ctx);
        if (body_type == nullptr) {
            std::cout << "Failed to generate type for abstraction body." << std::endl;
            delete param_type;
            return nullptr;
        }

        ctx.symtab.exit_level();

        return new Type(param_type, body_type);
    }

    Type *gen_identifier_expr_type(const IIdentifierExprAST *ident_node, CodeGenContext &ctx)
    {
        auto sym = ctx.symtab.lookup(ident_node->get_identifier());
        if (sym == nullptr) {
            std::cout << "Undefined identifier name: " << ident_node->get_identifier() << std::endl;
        }
        if (sym->type != SymbolTypes::SYMBOL_IDENTIFIER) {
            std::cout << "Identifier " << ident_node->get_identifier() << " is not a identifier for variable" << std::endl;
        }
        auto *info = dynamic_cast<VariableInfo *>(sym->info);
        return info->var_type;
    }

    Type *gen_application_expr_type(const IApplicationAST *app_node, CodeGenContext &ctx)
    {
        Type *func_type = gen_expr_type(app_node->function_expr(), ctx);
        for (auto *arg_ast : app_node->argument_expr()) {
            Type *arg_type = gen_expr_type(arg_ast, ctx);
            if (arg_type == nullptr) {
                std::cout << "Cannot parse type for argument" << std::endl;
                return nullptr;
            }

            Type *ret_type = func_type->apply(arg_type);
            if (ret_type == nullptr) {
                std::cout << "Function type " << func_type->repr() << " cannot be applied to argument type " << arg_type->repr() << std::endl;
                return nullptr;
            }
            
            func_type = ret_type;
        }
        return func_type;
    }

    Type *gen_expr_type(const IExprAST *expr_node, CodeGenContext &ctx)
    {
        switch (expr_node->get_expr_type()) {
            case ExpressionTypes::LITERAL: {
                const ILiteralExprAST *lit_node = dynamic_cast<const ILiteralExprAST *>(expr_node);
                if (lit_node == nullptr) {
                    return nullptr;
                }
                return gen_literal_expr_type(lit_node, ctx);
            }
            case ExpressionTypes::ABSTRACTION: {
                const IAbstractionAST *abst_node = dynamic_cast<const IAbstractionAST *>(expr_node);
                if (abst_node == nullptr) {
                    return nullptr;
                }
                return gen_abstraction_expr_type(abst_node, ctx);
            }
            case ExpressionTypes::SUCCESSOR: {
                const ISuccExprAST *succ_node = dynamic_cast<const ISuccExprAST *>(expr_node);
                if (succ_node == nullptr) {
                    return nullptr;
                }
                return gen_successor_expr_type(succ_node, ctx);
            }
            case ExpressionTypes::GENERAL_EXPR: {
                // 直接返回内部表达式的类型
                const IGeneralExprAST *gen_node = dynamic_cast<const IGeneralExprAST *>(expr_node);
                if (gen_node == nullptr) {
                    return nullptr;
                }
                return gen_expr_type(gen_node->inner_expr(), ctx);
            }
            case ExpressionTypes::PRIM_EXPR: {
                // 直接返回内部表达式的类型
                const IPrimExprAST *prim_node = dynamic_cast<const IPrimExprAST *>(expr_node);
                if (prim_node == nullptr) {
                    return nullptr;
                }
                return gen_expr_type(prim_node->inner_expr(), ctx);
            }
            case ExpressionTypes::IDENTIFIER: {
                const IIdentifierExprAST *ident_node = dynamic_cast<const IIdentifierExprAST *>(expr_node);
                if (ident_node == nullptr) {
                    return nullptr;
                }
                return gen_identifier_expr_type(ident_node, ctx);
            }
            case ExpressionTypes::APPLICATION: {
                const IApplicationAST *app_node = dynamic_cast<const IApplicationAST *>(expr_node);
                if (app_node == nullptr) {
                    return nullptr;
                }
                return gen_application_expr_type(app_node, ctx);
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

        Type *expr_type = gen_expr_type(value_expr, ctx);
        if (expr_type == nullptr) {
            std::cout << "Failed to parse type for value expression in let statement." << std::endl;
            delete var_type;
            return false;
        }

        std::cout << "Generated type for value expression: " << expr_type->repr() << std::endl;

        if (!type_match(var_type, expr_type)) {
            std::cout << "Type mismatch in let statement for variable '" << identifier << "'." << std::endl;
            delete var_type;
            delete expr_type;
            return false;
        }

        std::cout << "Type match confirmed for variable '" << identifier << "'." << std::endl;
        ctx.symtab.lookup_or_insert_variable(identifier, var_type);

        delete expr_type;

        return true;
    }

    bool gen_print_stmt(const IPrintStmtAST *print_node, CodeGenContext &ctx)
    {
        IExprAST *expr = print_node->expr();
        Type *expr_type = gen_expr_type(expr, ctx);
        if (expr_type == nullptr) {
            std::cout << "Failed to generate type for print expression." << std::endl;
            return false;
        }

        if (expr_type->kind != TypeKinds::TYPE_NAT) {
            std::cout << "Print expression must be of type Nat, but got: " << expr_type->repr() << std::endl;
            delete expr_type;
            return false;
        }

        std::cout << "Generated type for print expression: " << expr_type->repr() << std::endl;
        delete expr_type;

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
        for (const IStmtAST *stmt : program_node->statements()) {
            if (! gen_stmt(stmt, ctx)) {
                return false;
            }
        }
        return true;
    }
}