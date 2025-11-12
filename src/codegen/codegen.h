#pragma once

#include "codegen/symtab.h"
#include "parser/ast.h"

namespace rain {
    struct CodeGenContext {
        SymbolTable &symtab;

        CodeGenContext(SymbolTable &symtab)
            : symtab(symtab)
        {
        }
    };

    Type *gen_type(const ITypeAST *type_node, CodeGenContext &ctx);
    Type *gen_literal_expr_type(const ILiteralExprAST *literal_node, CodeGenContext &ctx); 
    Type *gen_successor_expr_type(const ISuccExprAST *succ_node, CodeGenContext &ctx);
    Type *gen_abstraction_expr_type(const IAbstractionAST *abst_node, CodeGenContext &ctx);
    Type *gen_identifier_expr_type(const IIdentifierExprAST *ident_node, CodeGenContext &ctx);
    Type *gen_application_expr_type(const IApplicationAST *app_node, CodeGenContext &ctx);
    Type *gen_expr_type(const IExprAST *expr_node, CodeGenContext &ctx);
    bool gen_let_stmt(const ILetStmtAST *let_node, CodeGenContext &ctx);
    bool gen_print_stmt(const IPrintStmtAST *print_node, CodeGenContext &ctx);
    bool gen_stmt(const IStmtAST *stmt_node, CodeGenContext &ctx);
    bool gen_program(const IProgramAST *program_node, CodeGenContext &ctx);
}