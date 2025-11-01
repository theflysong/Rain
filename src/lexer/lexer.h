#pragma once

#include "util/util.h"
#include "file/posinfo.h"
#include "lexer/token_type.h"

namespace rain {
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

        std::string repr() const {
            return std::format("Token(type: {}, content: '{}', pos: {}:{}:{})",
                               static_cast<short>(type),
                               content,
                               pos != nullptr ? pos->path : "<unknown>",
                               pos != nullptr ? pos->line : -1,
                               pos != nullptr ? pos->column : -1);
        }
    };

    void initialize_lexer_phase();
    void terminate_lexer_phase();

    class Lexer {
    private:
        int token_ptr;

        bytebuffer buffer;

        void produce(int required = 1);

    public:
        std::vector<Token *> token_sequence;
        struct position {
            std::string path;
            int line;
            int column;
        } pos;

        Lexer(bytebuffer buf, std::string path = "")
            : token_sequence(), token_ptr(0), buffer(std::move(buf)), pos({path, 1, 1})
        {
            token_sequence.reserve(1000);
            initialize_lexer_phase();
        }

        ~Lexer() {
            terminate_lexer_phase();
        }

        [[nodiscard]] bool valid_ptr(int ptr) const {
            return ptr >= 0 && ptr < token_sequence.size();
        }

        bool rewind(int decreasement) {
            if (! valid_ptr(token_ptr - decreasement)) {
                return false;
            }

            token_ptr -= decreasement;
            return true;
        }

        bool ahead(int increasement) {
            if (! valid_ptr(token_ptr + increasement)) {
                return false;
            }

            token_ptr += increasement;
            return true;
        }
        
        [[nodiscard]] Token *peer() {
            if (token_ptr < 0)
                throw std::out_of_range("The pointer of lexer is smaller than 0 when peering a token");

            if (token_ptr >= token_sequence.size()) {
                produce();
            }

            Token *tok = token_sequence.at(token_ptr);
            return tok;
        }
        
        Token *next() {
            token_ptr += 1;
            return peer();
        }
    };

    static inline PosInfo *makepos(Lexer::position &pos) {
        return new PosInfo(pos.path, pos.line, pos.column);
    }

    struct LexError {
        std::string msg;
        PosInfo *pos;

        LexError(std::string msg, PosInfo *pos)
            : msg(msg), pos(pos)
        {
        }

        LexError(std::string msg, Lexer::position &pos)
            : LexError(msg, makepos(pos))
        {
        }

        LexError(std::string expected, char got, PosInfo *pos)
            : LexError(std::format("Expected {}, but got {}", expected, got), pos)
        {
        }

        LexError(std::string expected, char got, Lexer::position &pos)
            : LexError(expected, got, makepos(pos))
        {
        }

        LexError()
            : LexError("", nullptr)
        {
        }
    };
}