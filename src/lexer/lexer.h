#pragma once

#include "util/util.h"
#include "file/posinfo.h"

namespace rain {
#include "lexer/token_type.h"
    struct Token {
        static mem::Pool<Token> pool;

        TokenType type;
        std::string content;
        PosInfo *pos;

        Token(TokenType type, std::string content, PosInfo *pos = nullptr)
            : type(type), content(content), pos(pos)
        {
            pool.mark(this);
        }
    };
    
    class Lexer;
}

namespace rain{
    class Lexer {
    private:
        std::vector<Token *> token_sequence;
        int token_ptr;

        void produce(int required = 1);

    public:
        Lexer()
            : token_sequence(1000)
        {
        }

        ~Lexer() = default;

        [[nodiscard]] bool valid_ptr(int ptr) const {
            return ptr > 0 && ptr < token_sequence.size();
        }

        bool rewind(int decreasement) {
            if (! valid_ptr(token_ptr - decreasement)) {
                return false;
            }

            token_ptr -= decreasement;
        }

        bool ahead(int increasement) {
            if (! valid_ptr(token_ptr + increasement)) {
                return false;
            }

            token_ptr += increasement;
        }
        
        Token *fetch() {
            if (token_ptr < 0)
                throw std::out_of_range("The pointer of lexer is smaller than 0 when fetching a token");

            if (token_ptr >= token_sequence.size()) {
                produce();
            }

            Token *tok = token_sequence.at(token_ptr);
            ahead(1);

            return tok;
        }
    };
}