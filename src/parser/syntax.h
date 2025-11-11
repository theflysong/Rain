#include "parser/ast.h"

namespace rain {
    // 前向声明
    class ExprNode;

    template <typename T>
    constexpr const IExpr *IExpr_cast(const T *node) {
        return (const IExpr *)(node);
    }

    // 前向声明
    class TypeNode;
    class ExprNode;

    class LiteralNode :
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

    class IdentifierNode :
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
    };

    class TypeNode :
    public Choice<
        Connect<
            Terminal<TokenType::SIGN_MUL>,
            Closure<Connect<
                Terminal<TokenType::SIGN_POINTER>,
                TypeNode
            >>
        >,
        Connect<
            Terminal<TokenType::SIGN_LPAREN>,
            TypeNode,
            Terminal<TokenType::SIGN_RPAREN>,
            Closure<Connect<
                Terminal<TokenType::SIGN_POINTER>,
                TypeNode
            >>
        >
    > {
    public:
        using Choice::Choice;

        template <typename Base>
        TypeNode(Base *base) : Choice(*base) {
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

    class AbstractionNode :
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
    };
    class ApplicationNode :
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
    };

    class InduceExprNode :
    public Connect<
        DiscardTerminal<TokenType::SIGN_AT>,
        IdentifierNode,
        DiscardTerminal<TokenType::SIGN_LBRACKET>,
        ExprNode,
        DiscardTerminal<TokenType::SIGN_COMMA>,
        AbstractionNode,
        DiscardTerminal<TokenType::SIGN_RBRACKET>
    > {
    public:
        using Connect::Connect;

        template <typename Base>
        InduceExprNode(Base *base) : Connect(*base) {
        }

        static bool lookahead(TokenIter begin, TokenIter end) {
            return Connect::lookahead(begin, end);
        }

        static ParseResult<InduceExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = Connect::parse(begin, end);
            if (!result.success) {
                return ParseResult<InduceExprNode>::failed(end);
            }
            return ParseResult<InduceExprNode>(result.success,
                                              new InduceExprNode(result.val),
                                              result.end);
        }
    };

    class PrimExprNode :
    public Choice<
        IdentifierNode,
        LiteralNode,
        Connect<
            DiscardTerminal<TokenType::SIGN_LPAREN>,
            ExprNode,
            DiscardTerminal<TokenType::SIGN_RPAREN>
        >,
        InduceExprNode,
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
    };

    class SuccExprNode :
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
    };

    class ExprNode :
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
    };

    class LetStmtNode :
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
    };

    class ProgramNode :
    public Closure<LetStmtNode> {
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
    };
}