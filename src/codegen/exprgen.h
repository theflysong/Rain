/**
 * Generate the computation DAG from the AST.
 */

#pragma once

#include "parser/ast.h"
#include "codegen/compdag.h"

namespace rain {
    CompNode *general_expr_gen(const IExpr *expr, CompDAG &dag);
    CompNode *literal_expr_gen(const ILiteralExpr *expr, CompDAG &dag);
    CompNode *member_access_expr_gen(const IMemberAccessExpr *expr, CompDAG &dag);
    CompNode *primary_expr_gen(const IPrimaryExpr *primary, CompDAG &dag);
    CompNode *mul_expr_gen(const IMulExpr *mul, CompDAG &dag);
    CompNode *add_expr_gen(const IAddExpr *expr, CompDAG &dag);
}
