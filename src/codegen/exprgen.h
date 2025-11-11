/**
 * Generate the computation DAG from the AST.
 */

#pragma once

#include "parser/ast.h"
#include "codegen/compdag.h"

namespace rain {
    CompNode *general_expr_gen(const IExpr *expr, CompDAG &dag, CodeGenContext &ctx);
    CompNode *literal_expr_gen(const ILiteralExpr *expr, CompDAG &dag, CodeGenContext &ctx);
    CompNode *member_access_expr_gen(const IMemberAccessExpr *expr, CompDAG &dag, CodeGenContext &ctx);
    CompNode *primary_expr_gen(const IPrimaryExpr *primary, CompDAG &dag, CodeGenContext &ctx);
    CompNode *mul_expr_gen(const IMulExpr *mul, CompDAG &dag, CodeGenContext &ctx);
    CompNode *add_expr_gen(const IAddExpr *expr, CompDAG &dag, CodeGenContext &ctx);
}
