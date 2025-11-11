#include "codegen/exprgen.h"

namespace rain {
    CompNode *general_expr_gen(const IExpr *expr, CompDAG &dag, CodeGenContext &ctx) {
        switch (expr->get_expr_type()) {
            case ExpressionTypes::LITERAL:
                return literal_expr_gen(static_cast<const ILiteralExpr *>(expr), dag, ctx);
            case ExpressionTypes::MEMBER_ACCESS:
                return member_access_expr_gen(static_cast<const IMemberAccessExpr *>(expr), dag, ctx);
            case ExpressionTypes::PRIMARY:
                return primary_expr_gen(static_cast<const IPrimaryExpr *>(expr), dag, ctx);
            case ExpressionTypes::MUL:
                return mul_expr_gen(static_cast<const IMulExpr *>(expr), dag, ctx);
            case ExpressionTypes::ADD:
                return add_expr_gen(static_cast<const IAddExpr *>(expr), dag, ctx);
            default:
                return nullptr;
        }
    }

    CompNode *literal_expr_gen(const ILiteralExpr *expr, CompDAG &dag, CodeGenContext &ctx) {
        SymbolNode *node = dag.get_sym(expr->get_literal()->content);
        if (node == nullptr) {
            SymbolEntry *symbol = ctx.symtab.lookup_or_insert_literal(expr->get_literal());
            node = new SymbolNode(dag, symbol);
            dag.add_node(node);
        }
        return node;
    }

    CompNode *member_access_expr_gen(const IMemberAccessExpr *expr, CompDAG &dag, CodeGenContext &ctx) {
        SymbolNode *node = dag.get_sym(expr->get_member_name());
        if (node == nullptr) {
            SymbolEntry *symbol = ctx.symtab.lookup_or_insert_variable(expr->get_member_name());
            node = new SymbolNode(dag, symbol);
            dag.add_node(node);
        }
        return node;
    }

    CompNode *primary_expr_gen(const IPrimaryExpr *primary, CompDAG &dag, CodeGenContext &ctx) {
        const auto &sub_exprs = primary->get_sub_exprs();
        if (sub_exprs.size() != 1) {
            return nullptr;
        }
        return general_expr_gen(sub_exprs[0], dag, ctx);
    }

    CompNode *mul_expr_gen(const IMulExpr *mul, CompDAG &dag, CodeGenContext &ctx) {
        const auto &sub_exprs = mul->get_sub_exprs();
        const auto &operators = mul->get_operators();
        if (sub_exprs.size() != operators.size() + 1) {
            return nullptr;
        }

        CompNode *left = general_expr_gen(sub_exprs[0], dag, ctx);
        for (size_t i = 0; i < operators.size(); ++i) {
            CompNode *right = general_expr_gen(sub_exprs[i + 1], dag, ctx);
            DAGNodeTypes node_type;
            switch (operators[i]) {
                case OperatorTypes::MUL:
                    node_type = DAGNodeTypes::MUL;
                    break;
                case OperatorTypes::DIV:
                    node_type = DAGNodeTypes::DIV;
                    break;
                case OperatorTypes::MOD:
                    node_type = DAGNodeTypes::MOD;
                    break;
                default:
                    return nullptr;
            }
            CompNode *parent = new CompNode(dag, node_type, {left, right});
            dag.add_node(parent);
            left = parent;
        }
        return left;
    }

    CompNode *add_expr_gen(const IAddExpr *expr, CompDAG &dag, CodeGenContext &ctx) {
        const auto &sub_exprs = expr->get_sub_exprs();
        const auto &operators = expr->get_operators();
        if (sub_exprs.size() != operators.size() + 1) {
            return nullptr;
        }

        CompNode *left = general_expr_gen(sub_exprs[0], dag, ctx);
        for (size_t i = 0; i < operators.size(); ++i) {
            CompNode *right = general_expr_gen(sub_exprs[i + 1], dag, ctx);
            DAGNodeTypes node_type;
            switch (operators[i]) {
                case OperatorTypes::ADD:
                    node_type = DAGNodeTypes::ADD;
                    break;
                case OperatorTypes::SUB:
                    node_type = DAGNodeTypes::SUB;
                    break;
                default:
                    return nullptr;
            }
            CompNode *parent = new CompNode(dag, node_type, {left, right});
            dag.add_node(parent);
            left = parent;
        }
        return left;
    }
}