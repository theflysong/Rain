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

static bool isidalpha(char ch) {
    return isalpha(ch) || ch == '_';
}

static bool isidalnum(char ch) {
    return isalnum(ch) || ch == '_';
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
    assert(__match__(buf, "0b"));
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

struct __lex_trie__ {
    static mem::Pool<__lex_trie__> pool;

    TokenType type;
    map<char, __lex_trie__*> children;

    __lex_trie__(TokenType type = TokenType::NONE)
        : type(type), children()
    {
        pool.mark(this);
    }
} *__trie_root__ = nullptr;

mem::Pool<__lex_trie__> __lex_trie__::pool = mem::Pool<__lex_trie__>(100);

static void __add_keyword__(const std::string &keyword, TokenType type) {
    __lex_trie__ *node = __trie_root__;
    for (char ch : keyword) {
        if (! node->children.contains(ch)) {
            node->children[ch] = new __lex_trie__();
        }
        node = node->children[ch];
    }
    node->type = type;
}

static TokenType __lookup_keyword__(std::string &str) {
    __lex_trie__ *node = __trie_root__;
    for (char ch : str) {
        if (! node->children.contains(ch)) {
            return TokenType::NONE;
        }
        node = node->children[ch];
    }
    return node->type;
}

static void __identifier_or_keyword__(bytebuffer &buf, Lexer::position &pos, std::vector<LexError> &err) {
    char lookahead1 = buf.peer();

    assert(isidalpha(lookahead1));

    while (isidalnum(lookahead1)) {
        buf.ahead(1);
        pos.column += 1;

        lookahead1 = buf.peer();
    }
}

static void __literal_string__(bytebuffer &buf, Lexer::position &pos, std::vector<LexError> &err) {
    enum {
        INITIAL         = 0,
        TERMINATE       = 1,
        CHARS           = 2,
        ESCAPE          = 3
    } state = INITIAL;
    
    assert(buf.peer() == '"');

    while (state != TERMINATE) {
        char lookahead = buf.peer(0);
        int aheading = 1;

        switch (state)
        {
        case INITIAL: {
            // Consume opening "
            buf.ahead(1);
            pos.column += 1;
            state = CHARS;
            break;
        }
        case CHARS: {
            if (lookahead == '"') {
                state = TERMINATE;
                break;
            }
            else if (lookahead == '\\') {
                state = ESCAPE;
                break;
            }
            else if (lookahead == '\0' || lookahead == '\n') {
                err.emplace_back("Closing double quote (\") for string literal", lookahead, pos);
                return;
            }
            break;
        }
        case ESCAPE: {
            char lookahead_2 = buf.peer(1);
            switch (lookahead_2) {
            case 'a': case 'b': case 'f': case 'n':
            case 'r': case 't': case 'v': case '?':
            case '\\': case '\'': case '"':
                aheading = 1;
                break;
            case 'x': case 'X': {
                // Hex escape sequence \xhh
                if (! isxdigit(buf.peer(2)) || ! isxdigit(buf.peer(3))) {
                    err.emplace_back("Valid hex digits for hex escape sequence", lookahead_2, pos);
                }
                aheading = 2;
                break;
            }
            case '0':
                if (! isodigit(buf.peer(2))) {
                    aheading = 1;
                    break;
                }
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7':
                // Octal escape sequence \ooo
                if (! isodigit(buf.peer(2)) || ! isodigit(buf.peer(3))) {
                    err.emplace_back("Valid octal digits for octal escape sequence", lookahead_2, pos);
                }
                aheading = 3;
                break;
            // Unicode escape sequence \uXXXX or \UXXXXXXXX
            case 'u': case 'U': {
                int hex_count = (lookahead_2 == 'u') ? 4 : 8;
                for (int i = 2 ; i < 2 + hex_count ; i ++) {
                    if (! isxdigit(buf.peer(i))) {
                        err.emplace_back("Valid hex digits for unicode escape sequence", buf.peer(i), pos);
                        break;
                    }
                }
                aheading = 1 + hex_count;
            }
            default:
                err.emplace_back("Valid escape sequence", lookahead_2, pos);
                break;
            }
            state = CHARS;
        }
        default:
            break;
        }

        buf.ahead(aheading);
        pos.column += aheading;
    }
}

static void __literal_char__(bytebuffer &buf, Lexer::position &pos, std::vector<LexError> &err) {
        enum {
        INITIAL   = 0,
        TERMINATE = 1,
        ESCAPE    = 2
    } state = INITIAL;
    
    assert(buf.peer() == '\'');

    while (state != TERMINATE) {
        char lookahead = buf.peer(0);
        int aheading = 1;

        switch (state)
        {
        case INITIAL: {
            // Consume opening '
            buf.ahead(1);
            pos.column += 1;
            if (lookahead == '\\') {
                state = ESCAPE;
            }
            else {
                state = TERMINATE;
            }
            break;
        }
        case ESCAPE: {
            char lookahead_2 = buf.peer(1);
            switch (lookahead_2) {
            case 'a': case 'b': case 'f': case 'n':
            case 'r': case 't': case 'v': case '?':
            case '\\': case '\'': case '"':
                aheading = 2;
                break;
            case 'x': case 'X': {
                // Hex escape sequence \xhh
                if (! isxdigit(buf.peer(2)) || ! isxdigit(buf.peer(3))) {
                    err.emplace_back("Valid hex digits for hex escape sequence", lookahead_2, pos);
                }
                aheading = 3;
                break;
            }
            case '0':
                if (! isodigit(buf.peer(2))) {
                    aheading = 2;
                    break;
                }
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7':
                // Octal escape sequence \ooo
                if (! isodigit(buf.peer(2)) || ! isodigit(buf.peer(3))) {
                    err.emplace_back("Valid octal digits for octal escape sequence", lookahead_2, pos);
                }
                aheading = 4;
                break;
            // Unicode escape sequence \uXXXX or \UXXXXXXXX
            case 'u': case 'U': {
                int hex_count = (lookahead_2 == 'u') ? 4 : 8;
                for (int i = 2 ; i < 2 + hex_count ; i ++) {
                    if (! isxdigit(buf.peer(i))) {
                        err.emplace_back("Valid hex digits for unicode escape sequence", buf.peer(i), pos);
                        break;
                    }
                }
                aheading = 1 + hex_count;
            }
            default:
                err.emplace_back("Valid escape sequence", lookahead_2, pos);
                break;
            }
            state = TERMINATE;
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
        type = lookahead2 == '>' ? TokenType::SIGN_POINTER : TokenType::SIGN_SUB;
        break;
    case '*':
        variable = true;
        type = TokenType::SIGN_MUL;
        break;
    case '/':
        variable = true;
        type = TokenType::SIGN_DIV;
        break;
    case '%':
        variable = true;
        type = TokenType::SIGN_MOD;
        break;
    case '|':
        variable = repeatable = true;
        type = TokenType::SIGN_OR;
        break;
    case '&':
        variable = repeatable = true;
        type = TokenType::SIGN_AND;
        break;
    case '^':
        variable = true;
        type = TokenType::SIGN_XOR;
        break;
    case '!':
        variable = true;
        type = TokenType::SIGN_NOT;
        break;
    case '#':
        type = TokenType::SIGN_SHARP;
        break;
    case '$':
        type = TokenType::SIGN_DOLLAR;
        break;
    case ',':
        type = TokenType::SIGN_COMMA;
        break;
    case ';':
        type = TokenType::SIGN_SEMICOLON;
        break;
    case ':':
        type = TokenType::SIGN_COLON;
        break;
    case '.':
        type = TokenType::SIGN_DOT;
        break;
    case '(':
        type = TokenType::SIGN_LPAREN;
        break;
    case ')':
        type = TokenType::SIGN_RPAREN;
        break;
    case '{':
        type = TokenType::SIGN_LBRACE;
        break;
    case '}':
        type = TokenType::SIGN_RBRACE;
        break;
    case '[':
        type = TokenType::SIGN_LBRACKET;
        break;
    case ']':
        type = TokenType::SIGN_RBRACKET;
        break;
    case '~':
        type = TokenType::SIGN_TILDE;
        break;
    case '?':
        type = TokenType::SIGN_QUESTION;
        break;
    case '@':
        type = TokenType::SIGN_AT;
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
    else if (isidalpha(lookahead1)) {
        type = TokenType::IDENTIFIER;
    }
    else if (lookahead1 == '"') {
        type = TokenType::LITERAL_STRING;
    }
    else if (lookahead1 == '\'') {
        type = TokenType::LITERAL_CHAR;
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
    case TokenType::IDENTIFIER:
        __identifier_or_keyword__(buf, pos, err);
        break;
    case TokenType::LITERAL_STRING:
        __literal_string__(buf, pos, err);
        break;
    case TokenType::LITERAL_CHAR:
        __literal_char__(buf, pos, err);
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
       if (type == TokenType::IDENTIFIER) {
           std::string ident_str = buf.slice(begin);
           TokenType keyword_type = __lookup_keyword__(ident_str);
           if (keyword_type != TokenType::NONE) {
               type = keyword_type;
           }
       }
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

static std::map<std::string, TokenType> __keyword_map__ = {
   {"if",       TokenType::KEYWORD_IF},
   {"else",     TokenType::KEYWORD_ELSE},
   {"for",      TokenType::KEYWORD_FOR},
   {"foreach",  TokenType::KEYWORD_FOREACH},
   {"while",    TokenType::KEYWORD_WHILE},
   {"return",   TokenType::KEYWORD_RETURN},
   {"break",    TokenType::KEYWORD_BREAK},
   {"continue", TokenType::KEYWORD_CONTINUE},
   {"do",       TokenType::KEYWORD_DO},
   {"byte",     TokenType::KEYWORD_BYTE},
   {"short",    TokenType::KEYWORD_SHORT},
   {"int",      TokenType::KEYWORD_INT},
   {"long",     TokenType::KEYWORD_LONG},
   {"float",    TokenType::KEYWORD_FLOAT},
   {"double",   TokenType::KEYWORD_DOUBLE},
   {"bool",     TokenType::KEYWORD_BOOL},
   {"char",     TokenType::KEYWORD_CHAR},
   {"void",     TokenType::KEYWORD_VOID},
   {"unsigned", TokenType::KEYWORD_UNSIGNED},
   {"signed",   TokenType::KEYWORD_SIGNED},
   {"trait",    TokenType::KEYWORD_TRAIT},
   {"struct",   TokenType::KEYWORD_STRUCT},
   {"import",   TokenType::KEYWORD_IMPORT},
   {"export",   TokenType::KEYWORD_EXPORT},
   {"const",    TokenType::KEYWORD_CONST},
   {"static",   TokenType::KEYWORD_STATIC},
   {"template", TokenType::KEYWORD_TEMPLATE},
   {"typedef",  TokenType::KEYWORD_TYPEDEF},
   {"fn",       TokenType::KEYWORD_FN},
   {"let",      TokenType::KEYWORD_LET},
   {"true",     TokenType::KEYWORD_TRUE},
   {"false",    TokenType::KEYWORD_FALSE},
   {"null",     TokenType::KEYWORD_NULL}
};

void rain::initialize_lexer_phase() {
    __trie_root__ = new __lex_trie__();

    for (const auto &pair : __keyword_map__) {
        std::string keyword = pair.first;
        TokenType type = pair.second;

        __add_keyword__(keyword, type);
    }
}

void rain::terminate_lexer_phase() {
    __lex_trie__::pool.cleanup();
}