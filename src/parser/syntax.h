#include "parser/ast.h"

namespace rain {
    // 前向声明
    class ExprNode;

    template <typename T>
    constexpr const IExpr *IExpr_cast(const T *node) {
        return (const IExpr *)(node);
    }

    class LiteralExprNode : public ILiteralExpr,
    public Choice<
        Terminal<TokenType::DEC_INTEGER>,
        Terminal<TokenType::HEX_INTEGER>,
        Terminal<TokenType::OCT_INTEGER>,
        Terminal<TokenType::BIN_INTEGER>,
        Terminal<TokenType::FLOAT>,
        Terminal<TokenType::LITERAL_STRING>,
        Terminal<TokenType::LITERAL_CHAR>
    > {
    public:
        using Choice::Choice;

        template <typename Base>
        LiteralExprNode(Base *base) : ILiteralExpr(), Choice(*base) {
        }
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return Choice::lookahead(begin, end);
        }
        
        static ParseResult<LiteralExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = Choice::parse(begin, end);
            return ParseResult<LiteralExprNode>(result.success, 
                                        new LiteralExprNode(result.val), 
                                        result.end);
        }

        virtual const Token * get_literal() const override {
            return std::visit(
                [](auto &&child) -> const Token* {
                    return child->token();
                },
                this->child());
        }
    };

    class MembleAccessExprNode : public IMemberAccessExpr,
    public Connect<
        Terminal<TokenType::IDENTIFIER>,
        Closure<Connect<
            DiscardTerminal<TokenType::SIGN_DOT>,
            Terminal<TokenType::IDENTIFIER>
        >>
    > {
    public:
        using Connect::Connect;

        template <typename Base>
        MembleAccessExprNode(Base *base) : IMemberAccessExpr(), Connect(*base) {
        }
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return Connect::lookahead(begin, end);
        }

        static ParseResult<MembleAccessExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = Connect::parse(begin, end);
            return ParseResult<MembleAccessExprNode>(result.success,
                                        new MembleAccessExprNode(result.val),
                                        result.end);
        }

        virtual std::vector<const IExpr *> get_sub_exprs() const override {
            return {};
        }

        virtual std::vector<OperatorTypes> get_operators() const override {
            return {};
        }

        virtual std::string get_member_name() const override {
            std::string names = std::get<0>(this->children())->token()->content;
            const auto &closure = std::get<1>(this->children());
            for (const auto &rest : closure->children()) {
                names += "." + std::get<1>(rest->children())->token()->content;
            }
            return names;
        }
    };

    using PrimaryExprProduction3 = Connect<
        DiscardTerminal<TokenType::SIGN_LPAREN>,
        ExprNode,
        DiscardTerminal<TokenType::SIGN_RPAREN>
    >;
    class PrimaryExprNode : public IPrimaryExpr,
    public Choice<
        LiteralExprNode,
        MembleAccessExprNode,
        PrimaryExprProduction3
    >  {
    public:
        using Choice::Choice;

        template <typename Base>
        PrimaryExprNode(Base *base) : IPrimaryExpr(), Choice(*base) {
        }
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return Choice::lookahead(begin, end);
        }
        
        static ParseResult<PrimaryExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = Choice::parse(begin, end);
            return ParseResult<PrimaryExprNode>(result.success, 
                                        new PrimaryExprNode(result.val), 
                                        result.end);
        }
        
        virtual std::vector<const IExpr *> get_sub_exprs() const override {
            const IExpr *subExpr = std::visit(
                [](auto &&child) -> const IExpr* {
                    if constexpr (
                        std::is_same_v<std::decay_t<decltype(child)>, PrimaryExprProduction3*>
                    ) {
                        return IExpr_cast(std::get<1>(child->children()));
                    } else {
                        return IExpr_cast(child);
                    }
                },
                this->child());
            return {subExpr};
        }

        virtual std::vector<OperatorTypes> get_operators() const override {
            return {};
        }
    };

    class MulExprNode : public IMulExpr,
    public Connect<
        PrimaryExprNode,
        Closure<Connect<
            Choice<
                Terminal<TokenType::SIGN_MUL>,
                Terminal<TokenType::SIGN_DIV>,
                Terminal<TokenType::SIGN_MOD>
            >,
            PrimaryExprNode
        >>
    > {
    public:
        using Connect::Connect;

        template <typename Base>
        MulExprNode(Base *base) : IMulExpr(), Connect(*base) {
        }
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return Connect::lookahead(begin, end);
        }
        
        static ParseResult<MulExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = Connect::parse(begin, end);
            return ParseResult<MulExprNode>(result.success, 
                                        new MulExprNode(result.val), 
                                        result.end);
        }

        virtual std::vector<const IExpr *> get_sub_exprs() const override {
            std::vector<const IExpr *> subExprs;
            subExprs.push_back(IExpr_cast(std::get<0>(this->children())));

            const auto &closure = std::get<1>(this->children());
            for (const auto &rest : closure->children()) {
                subExprs.push_back(IExpr_cast(std::get<1>(rest->children())));
            }
            return subExprs;
        }

        virtual std::vector<OperatorTypes> get_operators() const override {
            std::vector<OperatorTypes> ops;
            const auto &closure = std::get<1>(this->children());
            for (const auto &rest : closure->children()) {
                ops.push_back(std::visit(
                    [&](auto &&child) -> OperatorTypes {
                    switch (child->token()->type) {
                        case TokenType::SIGN_MUL:
                            return OperatorTypes::MUL;
                        case TokenType::SIGN_DIV:
                            return OperatorTypes::DIV;
                        case TokenType::SIGN_MOD:
                            return OperatorTypes::MOD;
                        default:
                            return OperatorTypes::NONE;
                    }}, std::get<0>(rest->children())->child()));

            }
            return ops;
        }
    };

    class AddExprNode :  public IAddExpr,
    public Connect<
        MulExprNode,
        Closure<Connect<
            Choice<
                Terminal<TokenType::SIGN_ADD>,
                Terminal<TokenType::SIGN_SUB>
            >,
            MulExprNode
        >>
    >
    {
    public:
        using Connect::Connect;

        template <typename Base>
        AddExprNode(Base *base) : IAddExpr(), Connect(*base) {
        }
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return Connect::lookahead(begin, end);
        }
        
        static ParseResult<AddExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = Connect::parse(begin, end);
            return ParseResult<AddExprNode>(result.success, 
                                        new AddExprNode(result.val), 
                                        result.end);
        }

        virtual std::vector<const IExpr *> get_sub_exprs() const override {
            std::vector<const IExpr *> subExprs;
            subExprs.push_back(IExpr_cast(std::get<0>(this->children())));
            
            const auto &closure = std::get<1>(this->children());
            for (const auto &rest : closure->children()) {
                subExprs.push_back(IExpr_cast(std::get<1>(rest->children())));
            }
            return subExprs;
        }

        virtual std::vector<OperatorTypes> get_operators() const override {
            std::vector<OperatorTypes> ops;
            const auto &closure = std::get<1>(this->children());
            for (const auto &rest : closure->children()) {
                ops.push_back(std::visit(
                    [&](auto &&child) -> OperatorTypes {
                    switch (child->token()->type) {
                        case TokenType::SIGN_ADD:
                            return OperatorTypes::ADD;
                        case TokenType::SIGN_SUB:
                            return OperatorTypes::SUB;
                        default:
                            return OperatorTypes::NONE;
                    }}, std::get<0>(rest->children())->child()));

            }
            return ops;
        }

        virtual ExpressionTypes get_expr_type() const override {
            return IAddExpr::get_expr_type();
        }
    };

    class ExprNode : public AddExprNode {
    public:
        using AddExprNode::AddExprNode;
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return AddExprNode::lookahead(begin, end);
        }
        
        static ParseResult<ExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = AddExprNode::parse(begin, end);
            return ParseResult<ExprNode>(result.success, 
                                        static_cast<ExprNode*>(result.val), 
                                        result.end);
        }
    };
}