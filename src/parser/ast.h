#pragma once

#include "lexer/lexer.h"
#include <type_traits>
#include <tuple>
#include <variant>

namespace rain {
    using TokenIter = std::vector<Token *>::const_iterator;

    template<typename T>
    struct ParseResult {
        bool success;
        T *val;
        TokenIter end;

        ParseResult(bool success, T *val, TokenIter end)
            : success(success), val(val), end(end)
        {
        } 

        static ParseResult<T> failed(TokenIter end) {
            return ParseResult<T>(false, nullptr, end);
        }
    };

    template<TokenType T>
    class TerminalNode {
    protected:
        const Token *_token;
    public:
        TerminalNode(const Token *token)
            : _token(token)
        {
        }
        const Token *token() const {
            return _token;
        }
        static bool lookahead(TokenIter begin, TokenIter end)
        {
            return begin != end && (*begin)->type == T;
        }
        static ParseResult<TerminalNode> parse(TokenIter begin, TokenIter end) {
            Token *tok = *begin;
            if (begin != end && (*begin)->type == T) {
                return ParseResult<TerminalNode>(true, new TerminalNode(*begin), begin + 1);
            }
            return ParseResult<TerminalNode>::failed(end);
        }
    };

    template<TokenType T>
    class DiscardTerminalNode {
    protected:
    public:
        static bool lookahead(TokenIter begin, TokenIter end)
        {
            return begin != end && (*begin)->type == T;
        }
        static ParseResult<DiscardTerminalNode> parse(TokenIter begin, TokenIter end) {
            Token *tok = *begin;
            if (begin != end && (*begin)->type == T) {
                return ParseResult<DiscardTerminalNode>(true, nullptr, begin + 1);
            }
            return ParseResult<DiscardTerminalNode>::failed(end);
        }
    };
    
    // 闭包节点 (0次或多次)
    template<typename T>
    class ClosureNode {
    public:
        ClosureNode(std::vector<T*>&& children) : _children(std::move(children)) {}
    
        const std::vector<T*>& children() const { return _children; }
    
        static bool lookahead(TokenIter begin, TokenIter end)
        {
            return T::lookahead(begin, end);
        }
        static ParseResult<ClosureNode<T>> parse(TokenIter begin, TokenIter end) {
            std::vector<T*> children;
            TokenIter current = begin;
        
            while (current != end) {
                if (! T::lookahead(current, end))
                    break;
                auto result = T::parse(current, end);
                if (! result.success)
                    break;

                children.push_back(result.val);
                current = result.end;
            }
        
            return ParseResult<ClosureNode<T>>(true, new ClosureNode<T>(std::move(children)), current);
    }

    private:
        std::vector<T*> _children;
    };
    
    // 连接节点 - 匹配一系列连续的节点
    template<typename... Nodes>
    class ConnectionNode {
    public:
        ConnectionNode(std::tuple<Nodes*...>&& children) : _children(std::move(children)) {}
        
        const std::tuple<Nodes*...>& children() const { return _children; }
        
        static bool lookahead(TokenIter begin, TokenIter end) {
            return lookahead_impl<0>(begin, end);
        }
        
        static ParseResult<ConnectionNode> parse(TokenIter begin, TokenIter end) {
            return parse_impl<0>(begin, end);
        }

    private:
        std::tuple<Nodes*...> _children;
        
        template<size_t Index>
        static bool lookahead_impl(TokenIter begin, TokenIter end) {
            if constexpr (Index < sizeof...(Nodes)) {
                using CurrentType = std::tuple_element_t<Index, std::tuple<Nodes...>>;
                if (!CurrentType::lookahead(begin, end))
                    return false;
                
                // 对于lookahead，我们只需要检查第一个元素
                // 如果需要更精确的lookahead，可以递归调用
                return true;
            }
            return true;
        }
        
        template<size_t Index>
        static ParseResult<ConnectionNode> parse_impl(TokenIter begin, TokenIter end) {
            if constexpr (Index == sizeof...(Nodes)) {
                // 所有节点都解析成功，创建ConnectionNode
                return ParseResult<ConnectionNode>(
                    true, 
                    new ConnectionNode(std::make_tuple(static_cast<Nodes*>(nullptr)...)), 
                    begin
                );
            } else {
                using CurrentType = std::tuple_element_t<Index, std::tuple<Nodes...>>;
                auto result = CurrentType::parse(begin, end);
                
                if (!result.success) {
                    return ParseResult<ConnectionNode>::failed(begin);
                }
                
                // 递归解析剩余节点
                auto next_result = parse_impl<Index + 1>(result.end, end);
                
                if (!next_result.success) {
                    delete result.val; // 清理当前节点
                    return ParseResult<ConnectionNode>::failed(begin);
                }
                
                // 构建完整的子节点元组
                auto new_tuple = build_children_tuple<Index>(
                    result.val, 
                    next_result.val->_children,
                    std::make_index_sequence<sizeof...(Nodes)>{}
                );
                
                delete next_result.val; // 删除临时的ConnectionNode
                
                return ParseResult<ConnectionNode>(
                    true,
                    new ConnectionNode(std::move(new_tuple)),
                    next_result.end
                );
            }
        }

        template<size_t Index, typename Tuple, size_t... Is>
        static auto build_children_tuple(auto* current_val, const Tuple& next_tuple, 
                                    std::index_sequence<Is...>) {
            return std::make_tuple(
                [&]() -> auto* {
                    if constexpr (Is == Index) {
                        return current_val;
                    } else {
                        return std::get<Is>(next_tuple);
                    }
                }()...
            );
        }
    };
    
    // 选择节点 - 从一系列候选节点中选择第一个能成功解析的
    template<typename... Nodes>
    class OptionsNode {
    public:
        OptionsNode(std::variant<Nodes*...>&& child, size_t index)
            : _child(std::move(child)), _index(index) {}

        const std::variant<Nodes*...>& child() const { return _child; }
        size_t index() const { return _index; }

        static bool lookahead(TokenIter begin, TokenIter end) {
            return lookahead_impl<0>(begin, end);
        }

        static ParseResult<OptionsNode> parse(TokenIter begin, TokenIter end) {
            return parse_impl<0>(begin, end);
        }

    private:
        std::variant<Nodes*...> _child;
        size_t _index;

        template<size_t Index>
        static bool lookahead_impl(TokenIter begin, TokenIter end) {
            if constexpr (Index < sizeof...(Nodes)) {
                using CurrentType = std::tuple_element_t<Index, std::tuple<Nodes...>>;
                if (CurrentType::lookahead(begin, end))
                    return true;
                return lookahead_impl<Index + 1>(begin, end);
            }
            return false;
        }

        template<size_t Index>
        static ParseResult<OptionsNode> parse_impl(TokenIter begin, TokenIter end) {
            if constexpr (Index == sizeof...(Nodes)) {
                return ParseResult<OptionsNode>::failed(end);
            } else {
                using CurrentType = std::tuple_element_t<Index, std::tuple<Nodes...>>;

                // 先通过 lookahead 快速排除不可能的候选
                if (!CurrentType::lookahead(begin, end)) {
                    return parse_impl<Index + 1>(begin, end);
                }

                auto result = CurrentType::parse(begin, end);
                if (result.success) {
                    std::variant<Nodes*...> v;
                    v.template emplace<Index>(result.val);
                    return ParseResult<OptionsNode>(true, new OptionsNode(std::move(v), Index), result.end);
                }

                // 尝试下一个候选
                return parse_impl<Index + 1>(begin, end);
            }
        }
    };
}