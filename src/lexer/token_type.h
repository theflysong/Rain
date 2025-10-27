#pragma once

#include <string>
#include <format>

namespace rain {
    enum class TokenType : short {
        NONE           = 0x0000,
        MASK_ERROR     = 0x4000,
        MASK_REPEAT    = 0x1000,
        MASK_SIGN      = 0x0800,
        MASK_VARIANT   = 0x0400,
        MASK_KEYWORD   = 0x0200,
        IDENTIFIER     = 0x0001,                        // id
        DEC_INTEGER    = 0x0002,                        // 0 | [1-9][0-9]*
        BIN_INTEGER    = 0x0003,                        // 0b[01]+
        HEX_INTEGER    = 0x0004,                        // 0x[0-9a-fA-F]+
        OCT_INTEGER    = 0x0005,                        // 0[0-7]+
        FLOAT          = 0x0006,                        // (0 | [1-9][0-9]*)? \. [0-9]+
        LITERAL_STRING = 0x0007,                        // ".*"
        LITERAL_CHAR   = 0x0008,                        // '.'
        SIGN_ASSIGN    = MASK_SIGN    | 0x0001,         // =
        SIGN_EQUAL     = MASK_REPEAT | SIGN_ASSIGN,     // ==
        SIGN_LT        = MASK_SIGN    | 0x0002,         // <
        SIGN_LTE       = MASK_VARIANT | SIGN_LT,        // <=
        SIGN_LSHIFT    = MASK_REPEAT  | SIGN_LT,        // <<
        SIGN_LSHIFTAS  = MASK_VARIANT | SIGN_LSHIFT,    // <<=
        SIGN_GT        = MASK_SIGN    | 0x0003,         // >
        SIGN_GTE       = MASK_VARIANT | SIGN_GT,        // >=
        SIGN_RSHIFT    = MASK_REPEAT  | SIGN_GT,        // >>
        SIGN_RSHIFTAS  = MASK_VARIANT | SIGN_RSHIFT,    // >>=
        SIGN_ADD       = MASK_SIGN    | 0x0004,         // +
        SIGN_INC       = MASK_REPEAT  | SIGN_ADD,       // ++
        SIGN_ADDAS     = MASK_VARIANT | SIGN_ADD,       // +=
        SIGN_SUB       = MASK_SIGN    | 0x0006,         // -
        SIGN_DEC       = MASK_REPEAT  | SIGN_SUB,       // --
        SIGN_SUBAS     = MASK_VARIANT | SIGN_SUB,       // -=
        SIGN_MUL       = MASK_SIGN    | 0x0007,         // *
        SIGN_MULAS     = MASK_VARIANT | SIGN_MUL,       // *=
        SIGN_DIV       = MASK_SIGN    | 0x0008,         // /
        SIGN_DIVAS     = MASK_VARIANT | SIGN_DIV,       // /=
        SIGN_MOD       = MASK_SIGN    | 0x0009,         // %
        SIGN_MODAS     = MASK_VARIANT | SIGN_MOD,       // %=
        KEYWORD_IF     = MASK_KEYWORD | 0x0001,
        KEYWORD_ELSE   = MASK_KEYWORD | 0x0002,
        ENDMARK        = 0x2000
    };
    
    TokenType operator|(TokenType lhs, TokenType rhs);
    TokenType operator|=(TokenType &lhs, TokenType rhs);
    bool operator&(TokenType lhs, TokenType rhs);

    // 为什么静态反射还不出哼啊啊啊啊啊啊啊
    // 我将在静态反射出现后补上这个tostring
    // 在这之前就靠AI了

    // 返回 TokenType 的名称字符串，仅使用 switch-case 实现
    inline const char* to_string(TokenType t) noexcept {
        switch (t) {
        case TokenType::NONE:           return "NONE";
        case TokenType::MASK_ERROR:     return "MASK_ERROR";
        case TokenType::MASK_REPEAT:    return "MASK_REPEAT";
        case TokenType::MASK_SIGN:      return "MASK_SIGN";
        case TokenType::MASK_VARIANT:   return "MASK_VARIANT";
        case TokenType::MASK_KEYWORD:   return "MASK_KEYWORD";
        case TokenType::IDENTIFIER:     return "IDENTIFIER";
        case TokenType::DEC_INTEGER:    return "DEC_INTEGER";
        case TokenType::BIN_INTEGER:    return "BIN_INTEGER";
        case TokenType::HEX_INTEGER:    return "HEX_INTEGER";
        case TokenType::OCT_INTEGER:    return "OCT_INTEGER";
        case TokenType::FLOAT:          return "FLOAT";
        case TokenType::LITERAL_STRING: return "LITERAL_STRING";
        case TokenType::LITERAL_CHAR:   return "LITERAL_CHAR";
        case TokenType::SIGN_ASSIGN:    return "SIGN_ASSIGN";
        case TokenType::SIGN_EQUAL:     return "SIGN_EQUAL";
        case TokenType::SIGN_LT:        return "SIGN_LT";
        case TokenType::SIGN_LTE:       return "SIGN_LTE";
        case TokenType::SIGN_LSHIFT:    return "SIGN_LSHIFT";
        case TokenType::SIGN_LSHIFTAS:  return "SIGN_LSHIFTAS";
        case TokenType::SIGN_GT:        return "SIGN_GT";
        case TokenType::SIGN_GTE:       return "SIGN_GTE";
        case TokenType::SIGN_RSHIFT:    return "SIGN_RSHIFT";
        case TokenType::SIGN_RSHIFTAS:  return "SIGN_RSHIFTAS";
        case TokenType::SIGN_ADD:       return "SIGN_ADD";
        case TokenType::SIGN_INC:       return "SIGN_INC";
        case TokenType::SIGN_ADDAS:     return "SIGN_ADDAS";
        case TokenType::SIGN_SUB:       return "SIGN_SUB";
        case TokenType::SIGN_DEC:       return "SIGN_DEC";
        case TokenType::SIGN_SUBAS:     return "SIGN_SUBAS";
        case TokenType::SIGN_MUL:       return "SIGN_MUL";
        case TokenType::SIGN_MULAS:     return "SIGN_MULAS";
        case TokenType::SIGN_DIV:       return "SIGN_DIV";
        case TokenType::SIGN_DIVAS:     return "SIGN_DIVAS";
        case TokenType::SIGN_MOD:       return "SIGN_MOD";
        case TokenType::SIGN_MODAS:     return "SIGN_MODAS";
        case TokenType::KEYWORD_IF:     return "KEYWORD_IF";
        case TokenType::KEYWORD_ELSE:   return "KEYWORD_ELSE";
        case TokenType::ENDMARK:        return "ENDMARK";
        default:                        return "UNKNOWN_TOKEN";
        }
    }
}

template<>
struct std::formatter<rain::TokenType> : std::formatter<std::string_view> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(rain::TokenType t, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "{}", to_string(t));
    }
};