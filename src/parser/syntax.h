#include "parser/ast.h"

namespace rain {

    template <typename T>
    constexpr const IExprAST *IExpr_cast(const T *node) {
        return (const IExprAST *)(node);
    }

    // 前向声明
    class TypeNode;
    class ExprNode;

    class LiteralNode : public ILiteralExprAST,
    public Terminal<TokenType::DEC_INTEGER>
    {
    public:
        using Terminal::Terminal;

        template <typename Base>
        LiteralNode(Base *base) : Terminal(*base) {
        }
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return Terminal::lookahead(begin, end);
        }

        virtual const Token *get_literal_token() const override {
            return this->token();
        }

        static ParseResult<LiteralNode> parse(TokenIter begin, TokenIter end) {
            auto result = Terminal::parse(begin, end);
            if (!result.success) {
                return ParseResult<LiteralNode>::failed(end);
            }
            return ParseResult<LiteralNode>(result.success, 
                                                new LiteralNode(result.val), 
                                                result.end);
        }
    };

    class IdentifierNode : public IIdentifierExprAST,
    public Terminal<TokenType::IDENTIFIER>
    {
    public:
        using Terminal::Terminal;

        template <typename Base>
        IdentifierNode(Base *base) : Terminal(*base) {
        }
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return Terminal::lookahead(begin, end);
        }

        static ParseResult<IdentifierNode> parse(TokenIter begin, TokenIter end) {
            auto result = Terminal::parse(begin, end);
            if (!result.success) {
                return ParseResult<IdentifierNode>::failed(end);
            }
            return ParseResult<IdentifierNode>(result.success, 
                                                new IdentifierNode(result.val), 
                                                result.end);
        }

        virtual std::string get_identifier() const override {
            return this->token()->lexeme;
        }
    };

    using ArrowToTypeBody = Connect<
        DiscardTerminal<TokenType::SIGN_POINTER>,
        TypeNode
        >;
    class TypeNode : public ITypeAST,
    public Choice<
        Connect<
            Terminal<TokenType::SIGN_MUL>,
            Closure<ArrowToTypeBody>
        >,
        Connect<
            DiscardTerminal<TokenType::SIGN_LPAREN>,
            TypeNode,
            DiscardTerminal<TokenType::SIGN_RPAREN>,
            Closure<ArrowToTypeBody>
        >
    > {
    public:
        using Choice::Choice;

        template <typename Base>
        TypeNode(Base *base) : ITypeAST(), Choice(*base) {
        }

        virtual std::vector<TypeTerm> flatten() const override {
            std::vector<TypeTerm> terms;
            std::vector<ArrowToTypeBody *> arrows;
            if (this->index() == 0) {
                // 第一种形式：* 后跟多个箭头
                auto production = std::get<0>(this->child());

                // 第一个是 *
                terms.push_back(TypeTerm{nullptr, true});
                arrows = std::get<1>(production->children())->children();
            } else if (this->index() == 1) {
                // 第二种形式：(type) 后跟多个箭头
                auto production = std::get<1>(this->child());
                // 第二个是 type
                auto inner_type = std::get<1>(production->children());

                terms.push_back(TypeTerm{inner_type, false});
                arrows = std::get<3>(production->children())->children();
            }
            for (auto *arrow : arrows) {
                auto arrow_type = std::get<1>(arrow->children());
                terms.push_back(TypeTerm{arrow_type, false});
            }
            return terms;
        }
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return Choice::lookahead(begin, end);
        }
        
        static ParseResult<TypeNode> parse(TokenIter begin, TokenIter end) {
            auto result = Choice::parse(begin, end);
            if (!result.success) {
                return ParseResult<TypeNode>::failed(end);
            }
            return ParseResult<TypeNode>(result.success, 
                                        new TypeNode(result.val), 
                                        result.end);
        }
    };

    class AbstractionNode : public IAbstractionAST,
    public Connect<
        DiscardTerminal<TokenType::SIGN_DOLLAR>, 
        DiscardTerminal<TokenType::SIGN_LPAREN>, 
        IdentifierNode,
        DiscardTerminal<TokenType::SIGN_COLON>,
        TypeNode,
        DiscardTerminal<TokenType::SIGN_RPAREN>,
        DiscardTerminal<TokenType::SIGN_DOT>,
        ExprNode
    > {
    public:
        using Connect::Connect;

        template <typename Base>
        AbstractionNode(Base *base) : Connect(*base) {
        }
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return Connect::lookahead(begin, end);
        }

        static ParseResult<AbstractionNode> parse(TokenIter begin, TokenIter end) {
            auto result = Connect::parse(begin, end);
            if (!result.success) {
                return ParseResult<AbstractionNode>::failed(end);
            }
            return ParseResult<AbstractionNode>(result.success, 
                                                new AbstractionNode(result.val), 
                                                result.end);
        }

        virtual std::string parameter_name() const override {
            auto id_node = std::get<2>(this->children());
            return id_node->token()->lexeme;
        }

        virtual ITypeAST *parameter_type() const override {
            return std::get<4>(this->children());
        }

        // 实现需要ExprNode的完整定义, 因此拖延到syntax.cpp中实现
        virtual const IExprAST *body_expr() const override;
    };
    class ApplicationNode : public IApplicationAST,
    public Connect<
        DiscardTerminal<TokenType::SIGN_SHARP>,
        IdentifierNode,
        Closure<Connect<
            DiscardTerminal<TokenType::SIGN_LPAREN>,
            ExprNode,
            DiscardTerminal<TokenType::SIGN_RPAREN>
        >>
    > {
    public:
        using Connect::Connect;

        template <typename Base>
        ApplicationNode(Base *base) : Connect(*base) {
        }

        static bool lookahead(TokenIter begin, TokenIter end) {
            return Connect::lookahead(begin, end);
        }

        static ParseResult<ApplicationNode> parse(TokenIter begin, TokenIter end) {
            auto result = Connect::parse(begin, end);
            if (!result.success) {
                return ParseResult<ApplicationNode>::failed(end);
            }
            return ParseResult<ApplicationNode>(result.success, 
                                                new ApplicationNode(result.val), 
                                                result.end);
        }

        virtual const IExprAST *function_expr() const override {
            return std::get<1>(this->children());
        }

        virtual std::vector<const IExprAST *> argument_expr() const override;
    };

    using PrimParenBody = Connect<
            DiscardTerminal<TokenType::SIGN_LPAREN>,
            ExprNode,
            DiscardTerminal<TokenType::SIGN_RPAREN>
        >;
    class PrimExprNode : public IPrimExprAST,
    public Choice<
        IdentifierNode,
        LiteralNode,
        PrimParenBody,
        ApplicationNode
    > {
    public:
        using Choice::Choice;

        template <typename Base>
        PrimExprNode(Base *base) : Choice(*base) {
        }

        static bool lookahead(TokenIter begin, TokenIter end) {
            return Choice::lookahead(begin, end);
        }

        static ParseResult<PrimExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = Choice::parse(begin, end);
            if (!result.success) {
                return ParseResult<PrimExprNode>::failed(end);
            }
            return ParseResult<PrimExprNode>(result.success,
                                              new PrimExprNode(result.val),
                                              result.end);
        }

        virtual const IExprAST *inner_expr() const override {
            if (this->index() == 2) {
                // 括号表达式，返回括号内的表达式
                auto paren_body = std::get<2>(this->child());
                return IExpr_cast(std::get<1>(paren_body->children()));
            }
            return std::visit([](auto&& arg) -> const IExprAST* {
                return IExpr_cast(arg);
            }, this->child());
        }
    };

    class SuccExprNode : public ISuccExprAST,
    public Connect<
        Closure<Terminal<TokenType::SIGN_INC>>,
        PrimExprNode
    > {
    public:
        using Connect::Connect;

        template <typename Base>
        SuccExprNode(Base *base) : Connect(*base) {
        }

        static bool lookahead(TokenIter begin, TokenIter end) {
            return Connect::lookahead(begin, end) || PrimExprNode::lookahead(begin, end);
        }

        static ParseResult<SuccExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = Connect::parse(begin, end);
            if (!result.success) {
                return ParseResult<SuccExprNode>::failed(end);
            }
            return ParseResult<SuccExprNode>(result.success,
                                              new SuccExprNode(result.val),
                                              result.end);
        }

        virtual const int num_succ() const override {
            auto incs = std::get<0>(this->children())->children();
            return static_cast<int>(incs.size());
        }

        virtual const IExprAST *sub_expr() const override {
            return IExpr_cast(std::get<1>(this->children()));
        }
    };

    class ExprNode : public IGeneralExprAST,
    public Choice<
        AbstractionNode,
        SuccExprNode
    > {
    public:
        using Choice::Choice;

        template <typename Base>
        ExprNode(Base *base) : Choice(*base) {
        }

        static bool lookahead(TokenIter begin, TokenIter end) {
            return Choice::lookahead(begin, end);
        }

        static ParseResult<ExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = Choice::parse(begin, end);
            if (!result.success) {
                return ParseResult<ExprNode>::failed(end);
            }
            return ParseResult<ExprNode>(result.success,
                                              new ExprNode(result.val),
                                              result.end);
        }

        virtual const IExprAST *inner_expr() const override {
            return std::visit([](auto&& arg) -> const IExprAST* {
                return IExpr_cast(arg);
            }, this->child());
        }
    };

    class LetStmtNode : public ILetStmtAST,
    public Connect<
        DiscardTerminal<TokenType::KEYWORD_LET>,
        IdentifierNode,
        DiscardTerminal<TokenType::SIGN_COLON>,
        TypeNode,
        DiscardTerminal<TokenType::SIGN_ASSIGN>,
        ExprNode,
        DiscardTerminal<TokenType::SIGN_SEMICOLON>
    > {
    public:
        using Connect::Connect;

        template <typename Base>
        LetStmtNode(Base *base) : Connect(*base) {
        }

        static bool lookahead(TokenIter begin, TokenIter end) {
            return Connect::lookahead(begin, end);
        }

        static ParseResult<LetStmtNode> parse(TokenIter begin, TokenIter end) {
            auto result = Connect::parse(begin, end);
            if (!result.success) {
                return ParseResult<LetStmtNode>::failed(end);
            }
            return ParseResult<LetStmtNode>(result.success,
                                              new LetStmtNode(result.val),
                                              result.end);
        }

        virtual std::string identifier() const override {
            auto id_node = std::get<1>(this->children());
            return id_node->token()->lexeme;
        }

        virtual ITypeAST *type_decl() const override {
            return std::get<3>(this->children());
        }

        virtual IExprAST *value_expr() const override  {
            return std::get<5>(this->children());
        }
    };

    class PrintStmtNode : public IPrintStmtAST,
    public Connect<
        DiscardTerminal<TokenType::SIGN_AT>,
        ExprNode,
        DiscardTerminal<TokenType::SIGN_SEMICOLON>
        > {
    public:
        using Connect::Connect;
        template <typename Base>
        PrintStmtNode(Base *base) : Connect(*base) {
        }

        static bool lookahead(TokenIter begin, TokenIter end) {
            return Connect::lookahead(begin, end);
        }

        static ParseResult<PrintStmtNode> parse(TokenIter begin, TokenIter end) {
            auto result = Connect::parse(begin, end);
            if (!result.success) {
                return ParseResult<PrintStmtNode>::failed(end);
            }
            return ParseResult<PrintStmtNode>(result.success,
                                              new PrintStmtNode(result.val),
                                              result.end);
        }

        virtual IExprAST *expr() const override {
            return std::get<1>(this->children());
        }
    };

    class ProgramNode : public IProgramAST,
    public Closure<Choice<
        LetStmtNode,
        PrintStmtNode>> {
    public:
        using Closure::Closure;

        template <typename Base>
        ProgramNode(Base *base) : Closure(*base) {
        }

        static bool lookahead(TokenIter begin, TokenIter end) {
            return Closure::lookahead(begin, end);
        }

        static ParseResult<ProgramNode> parse(TokenIter begin, TokenIter end) {
            auto result = Closure::parse(begin, end);
            if (!result.success) {
                return ParseResult<ProgramNode>::failed(end);
            }
            return ParseResult<ProgramNode>(result.success,
                                              new ProgramNode(result.val),
                                              result.end);
        }

        virtual std::vector<const IStmtAST*> statements() const override {
            std::vector<const IStmtAST *> stmts;
            for (auto *stmt_node : this->children()) {
                stmts.push_back(std::visit([](auto&& arg) -> const IStmtAST* {
                    return (const IStmtAST *)(arg);
                }, stmt_node->child()));
            }
            return stmts;
        }
    };
}