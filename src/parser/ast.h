#pragma once

#include "lexer/lexer.h"
#include <type_traits>
#include <tuple>
#include <variant>

namespace rain {
    using TokenIter = std::vector<Token *>::const_iterator;

    // 存储parse结果
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

    // 终结符
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

    // 终结符, 但是会将读到的符号抛弃
    // 例如 ( E ) 中将括号抛弃
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
            
            // 未读到头
            while (current != end) {
                // 先进行lookahead测试
                if (! T::lookahead(current, end))
                    break;
                auto result = T::parse(current, end);
                // parse失败
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
            return parse_impl<0>(begin, end, std::make_tuple());
        }

    private:
        std::tuple<Nodes*...> _children;
        
        // 从Index处进行lookahead
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
        
        // 从Index处进行parse
        // ParsedNode说明已经parse了多少
        template<size_t Index, typename... ParsedNodes>
        static ParseResult<ConnectionNode> parse_impl(TokenIter begin, TokenIter end, std::tuple<ParsedNodes...>&& current_tuple) {
            if constexpr (Index == sizeof...(Nodes)) {
                // 所有节点都解析成功，创建ConnectionNode
                return ParseResult<ConnectionNode>(
                    true, 
                    new ConnectionNode(std::move(current_tuple)),
                    begin
                );
            } else {
                // 提取现在应该使用的Node
                using CurrentType = std::tuple_element_t<Index, std::tuple<Nodes...>>;
                auto result = CurrentType::parse(begin, end);
                
                if (!result.success) {
                    // 清理已经解析的节点
                    cleanup_tuple(current_tuple);
                    return ParseResult<ConnectionNode>::failed(begin);
                }
                
                // 将当前解析的节点添加到元组中，继续解析下一个
                auto new_tuple = std::tuple_cat(
                    std::move(current_tuple),
                    std::make_tuple(result.val)
                );
                
                // 递归解析剩余节点
                return parse_impl<Index + 1, ParsedNodes..., CurrentType*>(result.end, end, std::move(new_tuple));
            }
        }
        
        // 辅助函数：清理元组中的节点指针
        template<typename... Ts>
        static void cleanup_tuple(const std::tuple<Ts*...>& tuple) {
            std::apply([](auto*... ptrs) {
                (delete ptrs, ...);
            }, tuple);
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