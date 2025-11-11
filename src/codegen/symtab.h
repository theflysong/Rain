#pragma once

#include <string>
#include <unordered_map>
#include "lexer/lexer.h"

namespace rain {
    enum class SymbolTypes {
        LITERAL_INTEGER,
        LITERAL_REAL,
        LITERAL_CHARACTER,
        LITERAL_STRING,
        SYMBOL_IDENTIFIER
    };

    class ISymbolInfo {
    public:
        SymbolTypes type;
        ISymbolInfo(SymbolTypes type) : type(type) {}
        virtual ~ISymbolInfo() = default;
        virtual std::string repr() const = 0;
    };

    class LiteralInfo : public ISymbolInfo {
        union {
            const long long int_value;
            const double real_value;
            const char char_value;
        };
        const std::string str_value;
    public:
        LiteralInfo(SymbolTypes type, long long val) : ISymbolInfo(type), int_value(val) {}
        LiteralInfo(SymbolTypes type, double val) : ISymbolInfo(type), real_value(val) {}
        LiteralInfo(SymbolTypes type, char val) : ISymbolInfo(type), char_value(val) {}
        LiteralInfo(SymbolTypes type, std::string &&val) : ISymbolInfo(type), str_value(std::move(val)) {}
        virtual ~LiteralInfo() {
        }
        virtual std::string repr() const override {
           switch (type) {
               case SymbolTypes::LITERAL_INTEGER:
                   return std::string("INTEGER: ") +std::to_string(int_value);
               case SymbolTypes::LITERAL_REAL:
                   return std::string("REAL: ") + std::to_string(real_value);
               case SymbolTypes::LITERAL_CHARACTER:
                   return std::string("CHARACTER: \'") + std::string(1, char_value) + std::string("\'");
               case SymbolTypes::LITERAL_STRING:
                   return std::string("STRING: \"") + str_value + std::string("\"");
               default:
                   return "<unknown literal>";
           }
        }
    };

    class VariableInfo : public ISymbolInfo {
    public:
        std::string identifier;
        LiteralInfo *info;
        VariableInfo(std::string identifier) 
            : ISymbolInfo(SymbolTypes::SYMBOL_IDENTIFIER), identifier(std::move(identifier)), info(nullptr)
        {
        }
        virtual ~VariableInfo() {
            if (info != nullptr) {
                delete info;
            }
        }
        virtual std::string repr() const override {
            if (info != nullptr) {
                return std::string("VARIABLE(LITERAL): ") + info->repr();
            }
            return "VARIABLE: " + identifier;
        }
    };
    
    struct SymbolEntry {
        std::string name;
        SymbolTypes type;
        ISymbolInfo *info;

        ~SymbolEntry() {
            if (info != nullptr) {
                delete info;
            }
        }
    };

    
    static inline SymbolEntry *create_literal_symbol(const Token *literal) {
        SymbolEntry *entry = new SymbolEntry();
        entry->name = literal->content;

        switch (literal->type) {
            case TokenType::DEC_INTEGER:
            case TokenType::BIN_INTEGER:
            case TokenType::HEX_INTEGER:
            case TokenType::OCT_INTEGER: {
                long long val = std::stoll(literal->content, nullptr, 0);
                entry->type = SymbolTypes::LITERAL_INTEGER;
                entry->info = new LiteralInfo(entry->type, val);
                break;
            }
            case TokenType::FLOAT: {
                double val = std::stod(literal->content);
                entry->type = SymbolTypes::LITERAL_REAL;
                entry->info = new LiteralInfo(entry->type, val);
                break;
            }
            case TokenType::LITERAL_CHAR: {
                char val = literal->content[1];
                entry->type = SymbolTypes::LITERAL_CHARACTER;
                entry->info = new LiteralInfo(entry->type, val);
                break;
            }
            case TokenType::LITERAL_STRING: {
                entry->info = new LiteralInfo(entry->type, literal->content.substr(1, literal->content.size() - 2));
                entry->type = SymbolTypes::LITERAL_STRING;
                break;
            }
            default:
                entry->info = nullptr;
                break;
        }

        return entry;
    }
    
    static inline SymbolEntry *create_literal_symbol(const std::string &name, SymbolTypes type, LiteralInfo *literal) {
        SymbolEntry *entry = new SymbolEntry();
        entry->name = name;
        entry->type = type;
        entry->info = literal;
        return entry;
    }

    static inline SymbolEntry *create_variable_symbol(const std::string &name) {
        SymbolEntry *entry = new SymbolEntry();
        entry->name = name;
        entry->type = SymbolTypes::SYMBOL_IDENTIFIER;
        entry->info = new VariableInfo(name);
        return entry;
    }

    struct SymbolTable {
        std::unordered_map<std::string, SymbolEntry*> table;

        ~SymbolTable() {
            for (auto &pair : table) {
                delete pair.second;
            }
            table.clear();
        }

        SymbolEntry *lookup(const std::string &name) {
            auto it = table.find(name);
            if (it != table.end()) {
                return it->second;
            }
            return nullptr;
        }

        void set_entry(SymbolEntry *entry) {
            auto *old = lookup(entry->name);
            if (old != nullptr) {
                delete old;
            }
            table[entry->name] = entry;
        }

        void insert(SymbolEntry *entry) {
            table[entry->name] = entry;
        }

        SymbolEntry *lookup_or_insert_literal(const Token *literal) {
            SymbolEntry *entry = lookup(literal->content);
            if (entry == nullptr) {
                entry = create_literal_symbol(literal);
                insert(entry);
            }
            return entry;
        }

        SymbolEntry *lookup_or_insert_variable(std::string identifier) {
            SymbolEntry *entry = lookup(identifier);
            if (entry == nullptr) {
                entry = create_variable_symbol(identifier);
                insert(entry);
            }
            return entry;
        }
    };
}