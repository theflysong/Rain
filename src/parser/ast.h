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

    class ITypeAST : public IASTNode {
    public:
        struct TypeTerm {
            ITypeAST *subtype;
            bool is_nat;
        };
        ITypeAST() : IASTNode() {
        }
        virtual std::vector<TypeTerm> flatten() const = 0;
        virtual ~ITypeAST() = default; 
    };

    enum class ExpressionTypes {
        IDENTIFIER,
        LITERAL,
        ABSTRACTION,
        APPLICATION,
        SUCCESSOR,
        GENERAL_EXPR,
        PRIM_EXPR
    };

    class IExprAST : public IASTNode {
    public:
        IExprAST() : IASTNode() {
        }
        virtual ~IExprAST() = default; 
        virtual ExpressionTypes get_expr_type() const = 0;
        virtual std::vector<const IExprAST *> get_sub_exprs() const = 0;
    };

    class ILiteralExprAST : public IExprAST {
    public:
        ILiteralExprAST() : IExprAST() {
        }
        virtual ~ILiteralExprAST() = default; 
        virtual ExpressionTypes get_expr_type() const override {
            return ExpressionTypes::LITERAL;
        }
        virtual std::vector<const IExprAST *> get_sub_exprs() const override {
            return {};
        }
        virtual const Token *get_literal_token() const = 0;
    };

    class IIdentifierExprAST : public IExprAST {
    public:
        IIdentifierExprAST() : IExprAST() {
        }
        virtual ~IIdentifierExprAST() = default; 
        virtual ExpressionTypes get_expr_type() const override {
            return ExpressionTypes::IDENTIFIER;
        }
        virtual std::vector<const IExprAST *> get_sub_exprs() const override {
            return {};
        }
        virtual std::string get_identifier() const = 0;
    };

    class IAbstractionAST : public IExprAST {
    public:
        IAbstractionAST() : IExprAST() {
        }
        virtual ~IAbstractionAST() = default; 
        virtual ExpressionTypes get_expr_type() const override {
            return ExpressionTypes::ABSTRACTION;
        }
        virtual std::vector<const IExprAST *> get_sub_exprs() const override {
            return {body_expr()};
        }
        virtual std::string parameter_name() const = 0;
        virtual ITypeAST *parameter_type() const = 0;
        virtual const IExprAST *body_expr() const = 0;
    };

    class ISuccExprAST : public IExprAST {
    public:
        ISuccExprAST() : IExprAST() {
        }
        virtual ~ISuccExprAST() = default; 
        virtual ExpressionTypes get_expr_type() const override {
            return ExpressionTypes::SUCCESSOR;
        }
        virtual const int num_succ() const = 0;
        virtual std::vector<const IExprAST *> get_sub_exprs() const override {
            return {sub_expr()};
        }
        virtual const IExprAST *sub_expr() const = 0;
    };

    class IApplicationAST : public IExprAST {
    public:
        IApplicationAST() : IExprAST() {
        }
        virtual ~IApplicationAST() = default; 
        virtual ExpressionTypes get_expr_type() const override {
            return ExpressionTypes::APPLICATION;
        }
        virtual std::vector<const IExprAST *> get_sub_exprs() const override {
            std::vector<const IExprAST *> args = argument_expr();
            args.insert(args.begin(), function_expr());
            return args;
        }
        virtual const IExprAST *function_expr() const = 0;
        virtual std::vector<const IExprAST *> argument_expr() const = 0;
    };

    class IGeneralExprAST : public IExprAST {
    public:
        IGeneralExprAST() : IExprAST() {
        }
        virtual ~IGeneralExprAST() = default; 
        virtual ExpressionTypes get_expr_type() const override {
            return ExpressionTypes::GENERAL_EXPR;
        }

        virtual std::vector<const IExprAST *> get_sub_exprs() const {
            return {inner_expr()};
        }

        virtual const IExprAST *inner_expr() const = 0;
    };

    class IPrimExprAST : public IExprAST {
    public:
        IPrimExprAST() : IExprAST() {
        }
        virtual ~IPrimExprAST() = default; 
        virtual ExpressionTypes get_expr_type() const override {
            return ExpressionTypes::PRIM_EXPR;
        }

        virtual std::vector<const IExprAST *> get_sub_exprs() const {
            return {inner_expr()};
        }
        
        virtual const IExprAST *inner_expr() const = 0;
    };

    class ILetStmtAST : public IASTNode {
    public:
        ILetStmtAST() : IASTNode() {
        }
        virtual ~ILetStmtAST() = default;
        virtual std::string identifier() const = 0;
        virtual ITypeAST *type_decl() const = 0;
        virtual IExprAST *value_expr() const = 0;
    };

    class IProgramAST : public IASTNode {
    public:
        IProgramAST() : IASTNode() {
        }
        virtual ~IProgramAST() = default;
        virtual std::vector<const ILetStmtAST *> let_statements() const = 0;
    };
}