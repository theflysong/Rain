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
        SIGN_OR        = MASK_SIGN    | 0x000A,         // |
        SIGN_ORAS      = MASK_VARIANT | SIGN_OR,        // |=
        SIGN_SWOR      = MASK_REPEAT  | SIGN_OR,        // ||
        SIGN_AND       = MASK_SIGN    | 0x000B,         // &
        SIGN_ANDAS     = MASK_VARIANT | SIGN_AND,       // &=
        SIGN_SWAND     = MASK_REPEAT  | SIGN_AND,       // &&
        SIGN_XOR       = MASK_SIGN    | 0x000C,         // ^
        SIGN_XORAS     = MASK_VARIANT | SIGN_XOR,       // ^=
        SIGN_NOT       = MASK_SIGN    | 0x000D,         // !
        SIGN_NEQ       = MASK_VARIANT | SIGN_NOT,       // !=
        SIGN_SHARP     = MASK_SIGN    | 0x000E,         // #
        SIGN_DOLLAR    = MASK_SIGN    | 0x000F,         // $
        SIGN_COMMA     = MASK_SIGN    | 0x0011,         // ,
        SIGN_DOT       = MASK_SIGN    | 0x0012,         // .
        SIGN_SEMICOLON = MASK_SIGN    | 0x0013,         // ;
        SIGN_COLON     = MASK_SIGN    | 0x0014,         // :
        SIGN_QUESTION  = MASK_SIGN    | 0x0015,         // ?
        SIGN_AT        = MASK_SIGN    | 0x0016,         // @
        SIGN_TILDE     = MASK_SIGN    | 0x0017,         // ~
        SIGN_QUOTE     = MASK_SIGN    | 0x0018,         // "
        SIGN_SINQUOTE  = MASK_SIGN    | 0x0019,         // '
        SIGN_LPAREN    = MASK_SIGN    | 0x0020,         // (
        SIGN_RPAREN    = MASK_SIGN    | 0x0021,         // )
        SIGN_LBRACE    = MASK_SIGN    | 0x0022,         // {
        SIGN_RBRACE    = MASK_SIGN    | 0x0023,         // }
        SIGN_LBRACKET  = MASK_SIGN    | 0x0024,         // [
        SIGN_RBRACKET  = MASK_SIGN    | 0x0025,         // ]
        SIGN_POINTER   = MASK_SIGN    | 0x0026,         // ->
        KEYWORD_IF       = MASK_KEYWORD | 0x0001,         // if
        KEYWORD_ELSE     = MASK_KEYWORD | 0x0002,         // else
        KEYWORD_FOR      = MASK_KEYWORD | 0x0003,         // for
        KEYWORD_FOREACH  = MASK_VARIANT | KEYWORD_FOR,    // foreach
        KEYWORD_WHILE    = MASK_KEYWORD | 0x0004,         // while
        KEYWORD_RETURN   = MASK_KEYWORD | 0x0005,         // return
        KEYWORD_BREAK    = MASK_KEYWORD | 0x0006,         // break
        KEYWORD_CONTINUE = MASK_KEYWORD | 0x0007,         // continue
        KEYWORD_DO       = MASK_KEYWORD | 0x0008,         // do
        KEYWORD_BYTE     = MASK_KEYWORD | 0x0009,         // byte
        KEYWORD_SHORT    = MASK_KEYWORD | 0x000A,         // short
        KEYWORD_INT      = MASK_KEYWORD | 0x000B,         // int
        KEYWORD_LONG     = MASK_KEYWORD | 0x000C,         // long
        KEYWORD_FLOAT    = MASK_KEYWORD | 0x000D,         // float
        KEYWORD_DOUBLE   = MASK_KEYWORD | 0x000E,         // double
        KEYWORD_BOOL     = MASK_KEYWORD | 0x000F,         // bool
        KEYWORD_CHAR     = MASK_KEYWORD | 0x0010,         // char
        KEYWORD_VOID     = MASK_KEYWORD | 0x0011,         // void
        KEYWORD_UNSIGNED = MASK_KEYWORD | 0x0012,         // unsigned
        KEYWORD_SIGNED   = MASK_KEYWORD | 0x0013,         // signed
        KEYWORD_TRAIT    = MASK_KEYWORD | 0x0014,         // trait
        KEYWORD_STRUCT   = MASK_KEYWORD | 0x0015,         // struct
        KEYWORD_IMPORT   = MASK_KEYWORD | 0x0016,         // import
        KEYWORD_EXPORT   = MASK_KEYWORD | 0x0017,         // export
        KEYWORD_CONST    = MASK_KEYWORD | 0x0018,         // const
        KEYWORD_STATIC   = MASK_KEYWORD | 0x0019,         // static
        KEYWORD_TEMPLATE = MASK_KEYWORD | 0x001A,         // template
        KEYWORD_TYPEDEF  = MASK_KEYWORD | 0x001B,         // typedef
        KEYWORD_FN       = MASK_KEYWORD | 0x001C,         // fn
        KEYWORD_LET      = MASK_KEYWORD | 0x001D,         // let
        KEYWORD_TRUE     = MASK_KEYWORD | 0x001E,         // true
        KEYWORD_FALSE    = MASK_KEYWORD | 0x001F,         // false
        KEYWORD_NULL     = MASK_KEYWORD | 0x0020,         // null
        ENDMARK          = 0x2000
    };
    
    TokenType operator|(TokenType lhs, TokenType rhs);
    TokenType operator|=(TokenType &lhs, TokenType rhs);
    bool operator&(TokenType lhs, TokenType rhs);

    // 为什么静态反射还不出哼啊啊啊啊啊啊啊
    // 我将在静态反射出现后补上这个tostring
    // 在这之前就靠AI了

    // 返回 TokenType 的名称字符串，仅使用 switch-case 实现
    inline const char* to_string(TokenType type) noexcept {
        switch (type) {
        case TokenType::NONE: return "NONE";
        case TokenType::MASK_ERROR: return "MASK_ERROR";
        case TokenType::MASK_REPEAT: return "MASK_REPEAT";
        case TokenType::MASK_SIGN: return "MASK_SIGN";
        case TokenType::MASK_VARIANT: return "MASK_VARIANT";
        case TokenType::MASK_KEYWORD: return "MASK_KEYWORD";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::DEC_INTEGER: return "DEC_INTEGER";
        case TokenType::BIN_INTEGER: return "BIN_INTEGER";
        case TokenType::HEX_INTEGER: return "HEX_INTEGER";
        case TokenType::OCT_INTEGER: return "OCT_INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::LITERAL_STRING: return "LITERAL_STRING";
        case TokenType::LITERAL_CHAR: return "LITERAL_CHAR";
        case TokenType::SIGN_ASSIGN: return "SIGN_ASSIGN";
        case TokenType::SIGN_EQUAL: return "SIGN_EQUAL";
        case TokenType::SIGN_LT: return "SIGN_LT";
        case TokenType::SIGN_LTE: return "SIGN_LTE";
        case TokenType::SIGN_LSHIFT: return "SIGN_LSHIFT";
        case TokenType::SIGN_LSHIFTAS: return "SIGN_LSHIFTAS";
        case TokenType::SIGN_GT: return "SIGN_GT";
        case TokenType::SIGN_GTE: return "SIGN_GTE";
        case TokenType::SIGN_RSHIFT: return "SIGN_RSHIFT";
        case TokenType::SIGN_RSHIFTAS: return "SIGN_RSHIFTAS";
        case TokenType::SIGN_ADD: return "SIGN_ADD";
        case TokenType::SIGN_INC: return "SIGN_INC";
        case TokenType::SIGN_ADDAS: return "SIGN_ADDAS";
        case TokenType::SIGN_SUB: return "SIGN_SUB";
        case TokenType::SIGN_DEC: return "SIGN_DEC";
        case TokenType::SIGN_SUBAS: return "SIGN_SUBAS";
        case TokenType::SIGN_MUL: return "SIGN_MUL";
        case TokenType::SIGN_MULAS: return "SIGN_MULAS";
        case TokenType::SIGN_DIV: return "SIGN_DIV";
        case TokenType::SIGN_DIVAS: return "SIGN_DIVAS";
        case TokenType::SIGN_MOD: return "SIGN_MOD";
        case TokenType::SIGN_MODAS: return "SIGN_MODAS";
        case TokenType::SIGN_OR: return "SIGN_OR";
        case TokenType::SIGN_ORAS: return "SIGN_ORAS";
        case TokenType::SIGN_SWOR: return "SIGN_SWOR";
        case TokenType::SIGN_AND: return "SIGN_AND";
        case TokenType::SIGN_ANDAS: return "SIGN_ANDAS";
        case TokenType::SIGN_SWAND: return "SIGN_SWAND";
        case TokenType::SIGN_XOR: return "SIGN_XOR";
        case TokenType::SIGN_XORAS: return "SIGN_XORAS";
        case TokenType::SIGN_NOT: return "SIGN_NOT";
        case TokenType::SIGN_NEQ: return "SIGN_NEQ";
        case TokenType::SIGN_SHARP: return "SIGN_SHARP";
        case TokenType::SIGN_DOLLAR: return "SIGN_DOLLAR";
        case TokenType::SIGN_COMMA: return "SIGN_COMMA";
        case TokenType::SIGN_DOT: return "SIGN_DOT";
        case TokenType::SIGN_SEMICOLON: return "SIGN_SEMICOLON";
        case TokenType::SIGN_COLON: return "SIGN_COLON";
        case TokenType::SIGN_QUESTION: return "SIGN_QUESTION";
        case TokenType::SIGN_AT: return "SIGN_AT";
        case TokenType::SIGN_TILDE: return "SIGN_TILDE";
        case TokenType::SIGN_QUOTE: return "SIGN_QUOTE";
        case TokenType::SIGN_SINQUOTE: return "SIGN_SINQUOTE";
        case TokenType::SIGN_LPAREN: return "SIGN_LPAREN";
        case TokenType::SIGN_RPAREN: return "SIGN_RPAREN";
        case TokenType::SIGN_LBRACE: return "SIGN_LBRACE";
        case TokenType::SIGN_RBRACE: return "SIGN_RBRACE";
        case TokenType::SIGN_LBRACKET: return "SIGN_LBRACKET";
        case TokenType::SIGN_RBRACKET: return "SIGN_RBRACKET";
        case TokenType::KEYWORD_IF: return "KEYWORD_IF";
        case TokenType::KEYWORD_ELSE: return "KEYWORD_ELSE";
        case TokenType::KEYWORD_FOR: return "KEYWORD_FOR";
        case TokenType::KEYWORD_FOREACH: return "KEYWORD_FOREACH";
        case TokenType::KEYWORD_WHILE: return "KEYWORD_WHILE";
        case TokenType::KEYWORD_RETURN: return "KEYWORD_RETURN";
        case TokenType::KEYWORD_BREAK: return "KEYWORD_BREAK";
        case TokenType::KEYWORD_CONTINUE: return "KEYWORD_CONTINUE";
        case TokenType::KEYWORD_DO: return "KEYWORD_DO";
        case TokenType::KEYWORD_BYTE: return "KEYWORD_BYTE";
        case TokenType::KEYWORD_SHORT: return "KEYWORD_SHORT";
        case TokenType::KEYWORD_INT: return "KEYWORD_INT";
        case TokenType::KEYWORD_LONG: return "KEYWORD_LONG";
        case TokenType::KEYWORD_FLOAT: return "KEYWORD_FLOAT";
        case TokenType::KEYWORD_DOUBLE: return "KEYWORD_DOUBLE";
        case TokenType::KEYWORD_BOOL: return "KEYWORD_BOOL";
        case TokenType::KEYWORD_CHAR: return "KEYWORD_CHAR";
        case TokenType::KEYWORD_VOID: return "KEYWORD_VOID";
        case TokenType::KEYWORD_UNSIGNED: return "KEYWORD_UNSIGNED";
        case TokenType::KEYWORD_SIGNED: return "KEYWORD_SIGNED";
        case TokenType::KEYWORD_TRAIT: return "KEYWORD_TRAIT";
        case TokenType::KEYWORD_STRUCT: return "KEYWORD_STRUCT";
        case TokenType::KEYWORD_IMPORT: return "KEYWORD_IMPORT";
        case TokenType::KEYWORD_EXPORT: return "KEYWORD_EXPORT";
        case TokenType::KEYWORD_CONST: return "KEYWORD_CONST";
        case TokenType::KEYWORD_STATIC: return "KEYWORD_STATIC";
        case TokenType::KEYWORD_TEMPLATE: return "KEYWORD_TEMPLATE";
        case TokenType::KEYWORD_TYPEDEF: return "KEYWORD_TYPEDEF";
        case TokenType::KEYWORD_FN: return "KEYWORD_FN";
        case TokenType::KEYWORD_LET: return "KEYWORD_LET";
        case TokenType::ENDMARK: return "ENDMARK";
        }
        return "UNKNOWN_TOKEN";
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