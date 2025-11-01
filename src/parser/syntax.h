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

        std::vector<PrimaryExprNode*> get_factors() const {
            std::vector<PrimaryExprNode*> factors;
            const auto &tpl = this->children();
            const PrimaryExprNode* first_factor = std::get<0>(tpl);
            factors.push_back(const_cast<PrimaryExprNode*>(first_factor));

            const auto &closure = std::get<1>(tpl);
            for (const auto &conn_node : closure->children()) {
                const auto &conn_tpl = conn_node->children();
                const PrimaryExprNode* factor = std::get<1>(conn_tpl);
                factors.push_back(const_cast<PrimaryExprNode*>(factor));
            }

            return factors;
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

        std::vector<MulExprNode*> get_terms() const {
            std::vector<MulExprNode*> terms;
            const auto &tpl = this->children();
            const MulExprNode* first_term = std::get<0>(tpl);
            terms.push_back(const_cast<MulExprNode*>(first_term));

            const auto &closure = std::get<1>(tpl);
            for (const auto &conn_node : closure->children()) {
                const auto &conn_tpl = conn_node->children();
                const MulExprNode* term = std::get<1>(conn_tpl);
                terms.push_back(const_cast<MulExprNode*>(term));
            }

            return terms;
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