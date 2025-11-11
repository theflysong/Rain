#pragma once

#include "parser/syntax_base.h"

namespace rain {
    class IASTNode {
    protected:
    public:
        static mem::Pool<IASTNode> pool;
        IASTNode() {
            pool.mark(this);
        }
        virtual ~IASTNode() = default;
    };

    enum class ExpressionTypes {
        LITERAL,
        MEMBER_ACCESS,
        PRIMARY,
        MUL,
        ADD
    };

    enum class OperatorTypes {
        NONE = 0,
        ADD, SUB, MUL, DIV, MOD,
        AND, OR, NOT, XOR, NEG,
        EQ, NEQ, LT, GT, LEQ, GEQ,
        LSH, RSH
    };

    class IExpr : public IASTNode {
    public:
        IExpr() : IASTNode() {
        }
        virtual ~IExpr() = default; 
        virtual ExpressionTypes get_expr_type() const = 0;
        virtual std::vector<const IExpr *> get_sub_exprs() const = 0;
        virtual std::vector<OperatorTypes> get_operators() const = 0;
    };

    class ILiteralExpr : public IExpr {
    public:
        ILiteralExpr() : IExpr() {
        }
        virtual ~ILiteralExpr() = default;

        virtual ExpressionTypes get_expr_type() const override {
            return ExpressionTypes::LITERAL;
        }
        
        virtual std::vector<const IExpr *> get_sub_exprs() const override {
            return {};
        }

        virtual std::vector<OperatorTypes> get_operators() const override {
            return {};
        }

        virtual const Token * get_literal() const = 0;
    };

    class IMemberAccessExpr : public IExpr {
    public:
        IMemberAccessExpr() : IExpr() {
        }
        virtual ~IMemberAccessExpr() = default;

        virtual ExpressionTypes get_expr_type() const override {
            return ExpressionTypes::MEMBER_ACCESS;
        }

        virtual std::string get_member_name() const = 0;
    };

    class IPrimaryExpr : public IExpr {
    public:
        IPrimaryExpr() : IExpr() {
        }
        virtual ~IPrimaryExpr() = default;

        virtual ExpressionTypes get_expr_type() const override {
            return ExpressionTypes::PRIMARY;
        }
    };

    class IMulExpr : public IExpr {
    public:
        IMulExpr() : IExpr() {
        }
        virtual ~IMulExpr() = default;

        virtual ExpressionTypes get_expr_type() const override {
            return ExpressionTypes::MUL;
        }
    };

    class IAddExpr : public IExpr {
    public:
        IAddExpr() : IExpr() {
        }
        virtual ~IAddExpr() = default;

        virtual ExpressionTypes get_expr_type() const override {
            return ExpressionTypes::ADD;
        }
    };
}