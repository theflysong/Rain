enum class TokenType {
    NONE           = 0x0000,
    MASK_SIGN      = 0x0800,
    MASK_VARIANT   = 0x0400,
    MASK_KEYWORD   = 0x0200,
    IDENTIFIER     = 0x0001,                   // id
    DEC_INTEGER    = 0x0002,    // 0 | [1-9][0-9]*
    BIN_INTEGER    = 0x0003,    // 0b[01]+
    HEX_INTEGER    = 0x0004,    // 0x[0-9a-fA-F]+
    OCT_INTEGER    = 0x0005,    // 0[0-7]+
    FLOAT          = 0x0006,    // (0 | [1-9][0-9]*)? \. [0-9]+
    LITERAL_STRING = 0x0007,    // ".*"
    LITERAL_CHAR   = 0x0008,     // '.'
    SIGN_LT        = MASK_SIGN | 0x0001,       // <
    SIGN_GT        = MASK_SIGN | 0x0002,       // >
    SIGN_LTE       = MASK_VARIANT | SIGN_LT,   // <=
    SIGN_GTE       = MASK_VARIANT | SIGN_GT,   // >=
    KEYWORD_IF     = MASK_KEYWORD | 0x0001,
    KEYWORD_ELSE   = MASK_KEYWORD | 0x0002
};