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
        IDENTIFIER,
        ABSTRACTION,
        APPLICATION
    };

    class IExpr : public IASTNode {
    public:
        IExpr() : IASTNode() {
        }
        virtual ~IExpr() = default; 
        virtual ExpressionTypes get_expr_type() const = 0;
        virtual std::vector<const IExpr *> get_sub_exprs() const = 0;
    };
}