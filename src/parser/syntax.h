#include "parser/ast.h"

namespace rain {
    // 前向声明
    class ExprNode;

    class LiteralNode : public OptionsNode<
        TerminalNode<TokenType::DEC_INTEGER>,
        TerminalNode<TokenType::HEX_INTEGER>,
        TerminalNode<TokenType::OCT_INTEGER>,
        TerminalNode<TokenType::BIN_INTEGER>,
        TerminalNode<TokenType::FLOAT>,
        TerminalNode<TokenType::LITERAL_STRING>,
        TerminalNode<TokenType::LITERAL_CHAR>
    > {
    public:
        using OptionsNode::OptionsNode;
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return OptionsNode::lookahead(begin, end);
        }
        
        static ParseResult<LiteralNode> parse(TokenIter begin, TokenIter end) {
            auto result = OptionsNode::parse(begin, end);
            return ParseResult<LiteralNode>(result.success, 
                                        static_cast<LiteralNode*>(result.val), 
                                        result.end);
        }
    };

    class PrimaryExprNode : public OptionsNode<
        LiteralNode,
        TerminalNode<TokenType::IDENTIFIER>,
        ConnectionNode<
            DiscardTerminalNode<TokenType::SIGN_LPAREN>,
            ExprNode,
            DiscardTerminalNode<TokenType::SIGN_RPAREN>
        >
    >  {
    public:
        using OptionsNode::OptionsNode;
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return OptionsNode::lookahead(begin, end);
        }
        
        static ParseResult<PrimaryExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = OptionsNode::parse(begin, end);
            return ParseResult<PrimaryExprNode>(result.success, 
                                        static_cast<PrimaryExprNode*>(result.val), 
                                        result.end);
        }
    };

    class MulExprNode : public ConnectionNode<
        PrimaryExprNode,
        ClosureNode<ConnectionNode<
            OptionsNode<
                TerminalNode<TokenType::SIGN_MUL>,
                TerminalNode<TokenType::SIGN_DIV>,
                TerminalNode<TokenType::SIGN_MOD>
            >,
            PrimaryExprNode
        >>
    > {
    public:
        using ConnectionNode::ConnectionNode;
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return ConnectionNode::lookahead(begin, end);
        }
        
        static ParseResult<MulExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = ConnectionNode::parse(begin, end);
            return ParseResult<MulExprNode>(result.success, 
                                        static_cast<MulExprNode*>(result.val), 
                                        result.end);
        }

        // 获取子primary expressions
        std::vector<PrimaryExprNode*> get_primary_exprs() const {
            std::vector<PrimaryExprNode*> primExprs;
            
            // 第一层: primExpr + (...)*
            const auto &tpl = this->children();
            // 将该层的primExpr加入
            primExprs.push_back(const_cast<PrimaryExprNode*>(std::get<0>(tpl)));

            // 第二层 (('*' | '/' | '%') primExpr)*
            const auto &closure = std::get<1>(tpl);
            for (const auto &conn_node : closure->children()) {
                // 取走第二个
                primExprs.push_back(
                    const_cast<PrimaryExprNode*>(
                        std::get<1>(conn_node->children())
                    ));
            }

            return primExprs;
        }
    };

    class AddExprNode : public ConnectionNode<
        MulExprNode,
        ClosureNode<ConnectionNode<
            OptionsNode<
                TerminalNode<TokenType::SIGN_ADD>,
                TerminalNode<TokenType::SIGN_SUB>
            >,
            MulExprNode
        >>
    >
    {
    public:
        using ConnectionNode::ConnectionNode;
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return ConnectionNode::lookahead(begin, end);
        }
        
        static ParseResult<AddExprNode> parse(TokenIter begin, TokenIter end) {
            auto result = ConnectionNode::parse(begin, end);
            return ParseResult<AddExprNode>(result.success, 
                                        static_cast<AddExprNode*>(result.val), 
                                        result.end);
        }

        std::vector<TokenType> get_operators() const {
            std::vector<TokenType> operators;
            
            // 获取闭包
            const auto &closure = std::get<1>(this->children());

            for (const auto &conn_node : closure->children()) {
                // 对闭包中的第一个node: variant<'+', '-'>
                // 使用泛型lambda捕获其类型
                operators.push_back(std::visit(
                        [](auto &&terminal) {
                          return terminal->token()->type;
                        },
                        std::get<0>(conn_node->children())->child()
                    ));
            }

            return operators;
        }

        std::vector<MulExprNode*> get_terms() const {
            std::vector<MulExprNode*> mulExprs;
            
            // 第一层: primExpr + (...)*
            const auto &tpl = this->children();
            // 将该层的primExpr加入
            mulExprs.push_back(const_cast<MulExprNode*>(std::get<0>(tpl)));

            // 第二层 (('*' | '/' | '%') primExpr)*
            const auto &closure = std::get<1>(tpl);
            for (const auto &conn_node : closure->children()) {
                // 取走第二个
                mulExprs.push_back(
                    const_cast<MulExprNode*>(
                        std::get<1>(conn_node->children())
                    ));
            }

            return mulExprs;
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