#include "lexer/lexer.h"
#include "lexer.h"

using namespace rain;

mem::Pool<Token> Token::pool = mem::Pool<Token>(1000);

static bool isodigit(char ch) {
    return '0' <= ch && ch <= '7';
}

static bool isbdigit(char ch) {
    return ch == '0' || ch == '1';
}

static bool __match__(bytebuffer &buf, std::string str) {
    bool flag = true;
    for (int i = 0 ; i < str.length() ; i ++) {
        flag &= (buf.peer(i) == str[i]);
    }
    return flag;
}

static void __skip__(bytebuffer &buf, Lexer::position &pos) {
    enum {
        INITIAL = 0,
        TERMINATE = 1,
        LINE_COMMENT = 2,
        MULLINE_COMMENT = 3
    } state = INITIAL;

    Lexer::position pos_begin = pos;

    while (state != TERMINATE) {
        char lookahead_1 = buf.peer(0), lookahead_2 = buf.peer(1);
        int aheading = 1;

        switch (state)
        {
        case INITIAL: {
            if (! isspace(lookahead_1)) {
                if (lookahead_1 != '/') {
                    state = TERMINATE;
                    aheading = 0;
                    break;
                }
                else if (lookahead_2 == '/') {
                    state = LINE_COMMENT;
                    aheading = 2;
                    break;
                }
                else if (lookahead_2 == '*') {
                    state = MULLINE_COMMENT;
                    aheading = 2;
                    break;
                }
                else {
                    state = TERMINATE;
                    aheading = 0;
                    break;
                }
            }
            break;
        }
        case LINE_COMMENT: {
            if (lookahead_1 == '\n') {
                state = INITIAL;
            }
            break;
        }
        case MULLINE_COMMENT: {
            if (lookahead_1 == '*' && lookahead_2 == '/') {
                state = INITIAL;
                aheading = 2;
            }
            break;
        }
        default: {
            break;
        }
        }

        buf.ahead(aheading);
        pos.column += aheading;

        if (lookahead_1 == '\n') {
            pos.column = 0;
            pos.line ++;
        }
    }

    printf("Discard from line %d col %d to line %d col %d\n", pos_begin.line, pos_begin.column, pos.line, pos.column);
}

static TokenType __dec_integer_or_float__(bytebuffer &buf, Lexer::position &pos, std::vector<LexError> &err) {
    enum {
        INITIAL = 0,
        TERMINATE = 1,
        DIGIT = 2,
        FLOATS = 3
    } state = INITIAL;

    TokenType type = TokenType::DEC_INTEGER;

    while (state != TERMINATE) {
        char lookahead = buf.peer(0);
        int aheading = 1;

        switch (state)
        {
        case INITIAL: {
            assert(isdigit(lookahead));
            if (lookahead == '0') {
                state = TERMINATE;
            }
            else {
                state = DIGIT;
            }
            break;
        }
        case DIGIT: {
            if (! isdigit(lookahead)) {
                if (lookahead == '.') {
                    type = TokenType::FLOAT;
                    state = FLOATS;
                    break;
                }
                state = TERMINATE;
                aheading = 0;
                break;
            }
        }
        case FLOATS: {
            if (! isdigit(lookahead)) {
                state = TERMINATE;
                aheading = 0;
                break;
            }
        }
        default:
            break;
        }

        buf.ahead(aheading);
        pos.column += aheading;
    }

    return type;
}

static void __hex_integer__(bytebuffer &buf, Lexer::position &pos, std::vector<LexError> &err) {
    assert(__match__(buf, "0x"));
    buf.ahead(2);

    enum {
        INITIAL = 0,
        TERMINATE = 1,
        DIGITS = 2
    } state = INITIAL;

    while (state != TERMINATE) {
        char lookahead = buf.peer(0);
        int aheading = 1;

        switch (state)
        {
        case INITIAL: {
            if (! isxdigit(lookahead)) {
                err.emplace_back("[0-9a-fA-F]", lookahead, pos);
                return;
            }
            state = DIGITS;
            break;
        }
        case DIGITS: {
            if (! isxdigit(lookahead)) {
                state = TERMINATE;
                aheading = 0;
            }
            break;
        }
        default:
            break;
        }

        buf.ahead(aheading);
        pos.column += aheading;
    }
}

static void __oct_integer__(bytebuffer &buf, Lexer::position &pos, std::vector<LexError> &err) {
    assert(__match__(buf, "0"));
    buf.ahead(1);

    enum {
        INITIAL = 0,
        TERMINATE = 1,
        DIGITS = 2
    } state = INITIAL;
        
    assert(isdigit(buf.peer(0)));

    while (state != TERMINATE) {
        char lookahead = buf.peer(0);
        int aheading = 1;

        switch (state)
        {
        case INITIAL: {
            if (! isodigit(lookahead)) {
                err.emplace_back("[0-7]", lookahead, pos);
            }
            // Continue process rest digits
            state = DIGITS;
            break;
        }
        case DIGITS: {
            if (! isdigit(lookahead)) {
                state = TERMINATE;
                aheading = 0;
                break;
            }

            if (! isodigit(lookahead)) {
                err.emplace_back("[0-7]", lookahead, pos);
                // Continue process rest digits
            }
            break;
        }
        default:
            break;
        }

        buf.ahead(aheading);
        pos.column += aheading;
    }
}

static void __bin_integer__(bytebuffer &buf, Lexer::position &pos, std::vector<LexError> &err) {
    assert(__match__(buf, "0x"));
    buf.ahead(2);

    enum {
        INITIAL = 0,
        TERMINATE = 1,
        DIGITS = 2
    } state = INITIAL;
        
    assert(isdigit(buf.peer(0)));

    while (state != TERMINATE) {
        char lookahead = buf.peer(0);
        int aheading = 1;

        switch (state)
        {
        case INITIAL: {
            if (! isbdigit(lookahead)) {
                err.emplace_back("[01]", lookahead, pos);
            }
            // Continue process rest digits
            state = DIGITS;
            break;
        }
        case DIGITS: {
            if (! isdigit(lookahead)) {
                state = TERMINATE;
                aheading = 0;
                break;
            }

            if (! isbdigit(lookahead)) {
                err.emplace_back("[01]", lookahead, pos);
                // Continue process rest digits
            }
            break;
        }
        default:
            break;
        }

        buf.ahead(aheading);
        pos.column += aheading;
    }
}

static TokenType __symbol__(bytebuffer &buf, Lexer::position &pos, std::vector<LexError> &err) {
    TokenType type = TokenType::NONE;
    char lookahead1 = buf.peer(), lookahead2 = buf.peer(1);

    int aheading = 0;

    bool variable = false, repeatable = false;

    switch (lookahead1)
    {
    case '=':
        repeatable = true;
        type = TokenType::SIGN_ASSIGN;
        break;
    case '>':
        variable = repeatable = true;
        type = TokenType::SIGN_GT;
        break;
    case '<':
        variable = repeatable = true;
        type = TokenType::SIGN_LT;
        break;
    case '+':
        variable = repeatable = true;
        type = TokenType::SIGN_ADD;
        break;
    case '-':
        variable = repeatable = true;
        type = TokenType::SIGN_SUB;
        break;
    case '*':
        variable = repeatable = true;
        type = TokenType::SIGN_MUL;
        break;
    case '/':
        variable = repeatable = true;
        type = TokenType::SIGN_DIV;
        break;
    case '%':
        variable = repeatable = true;
        type = TokenType::SIGN_MOD;
        break;
    default:
        break;
    }

    if (type == TokenType::NONE) {
        return type;
    }

    aheading ++;

    if (repeatable && (lookahead1 == lookahead2)) {
        type |= TokenType::MASK_REPEAT;
        variable = false;
        if (lookahead1 == '>' || lookahead2 == '<') {
            variable = true;
            lookahead2 = buf.peer(2);
        }
        aheading ++;
    }

    if (variable && lookahead2 == '=') {
        type |= TokenType::MASK_VARIANT;
        aheading ++;
    }

    buf.ahead(aheading);
    pos.column += aheading;
    return type;
}

static TokenType __nonsymbol__(bytebuffer &buf, Lexer::position &pos, std::vector<LexError> &err) {
    TokenType type = TokenType::NONE;
    char lookahead1 = buf.peer(), lookahead2 = buf.peer(1);
    
    if (isdigit(lookahead1))
    {
        if (lookahead1 == '0') {
            if (isdigit(lookahead2)) {
                type = TokenType::OCT_INTEGER;
            }
            else if (lookahead2 == 'x' || lookahead2 == 'X') {
                type = TokenType::HEX_INTEGER;
            }
            else if (lookahead2 == 'b' || lookahead2 == 'B') {
                type = TokenType::BIN_INTEGER;
            }
            else {
                type = TokenType::DEC_INTEGER;
            }
        }
        else {
            type = TokenType::DEC_INTEGER;
        }
    }

    switch(type) {
    case TokenType::DEC_INTEGER:
        // type could be dec_integer / float
        type = __dec_integer_or_float__(buf, pos, err);
        break;
    case TokenType::BIN_INTEGER:
        __bin_integer__(buf, pos, err);
        break;
    case TokenType::HEX_INTEGER:
        __hex_integer__(buf, pos, err);
        break;
    case TokenType::OCT_INTEGER:
        __oct_integer__(buf, pos, err);
        break;
    default:
        throw std::runtime_error("Unreachable Token Type!");
        break;
    }

    return type;
}

static Token *generate(bytebuffer &buf, Lexer::position &pos, std::vector<LexError> &err) {
    __skip__(buf, pos);
    size_t begin = buf.pos();
    Lexer::position begin_pos = pos;

    TokenType type = __symbol__(buf, pos, err);
    
    if (type == TokenType::NONE) {
       type =  __nonsymbol__(buf, pos, err);
    }

    if (type == TokenType::NONE) {
        err.emplace_back("Couldn't recognize any token", pos);
    }

    return new Token(type, buf.slice(begin), new PosInfo(begin_pos.path, begin_pos.line, begin_pos.column));
}

void Lexer::produce(int required)
{ 
    for (int i = 0 ; i < required ; i ++) {
        std::vector<LexError> err;

        Token *tok = generate(this->buffer, this->pos, err);
        
        if (! err.empty()) {
            tok->type |= TokenType::MASK_ERROR;
        }

        this->token_sequence.push_back(tok);

        for (const auto &e : err) {
            std::cout << std::format("[LEXER ERROR](file '{}', line {} col {}) {}", e.pos->path, e.pos->line, e.pos->column, e.msg) << std::endl;
        }
    }
}