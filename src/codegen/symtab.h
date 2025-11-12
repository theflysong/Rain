#pragma once

#include <string>
#include <unordered_map>
#include "lexer/lexer.h"
#include "codegen/type.h"

namespace rain {
    enum class SymbolTypes {
        NONE,
        LITERAL_INTEGER,
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
    public:
        long long int_value;
        LiteralInfo(SymbolTypes type, long long val) : ISymbolInfo(type), int_value(val) {}
        virtual ~LiteralInfo() {
        }
        virtual std::string repr() const override {
           switch (type) {
               case SymbolTypes::LITERAL_INTEGER:
                   return std::string("INTEGER: ") +std::to_string(int_value);
               default:
                   return "<unknown literal>";
           }
        }
    };

    class VariableInfo : public ISymbolInfo {
    public:
        std::string identifier;
        LiteralInfo *info;
        Type *var_type;
        
        VariableInfo(std::string identifier, Type *var_type)
            : ISymbolInfo(SymbolTypes::SYMBOL_IDENTIFIER), identifier(std::move(identifier)), info(nullptr), var_type(var_type)
        {
        }
        
        virtual ~VariableInfo() {
        }

        virtual std::string repr() const override {
            if (info != nullptr) {
                return std::string("VARIABLE(LITERAL): ") + info->repr();
            }
            if (var_type != nullptr) {
                return "VARIABLE(" + var_type->repr() + "): " + identifier;
            }
            return "VARIABLE: " + identifier;
        }
    };
    
    struct SymbolEntry {
        std::string name;
        SymbolTypes type;
        ISymbolInfo *info;
        int level;

        SymbolEntry(std::string name, SymbolTypes type, ISymbolInfo *info, int level) 
            : name(std::move(name)), type(type), info(info), level(level)
        {
        }

        ~SymbolEntry() {
            if (info != nullptr) {
                delete info;
            }
        }
    };
    
    static inline SymbolEntry *create_literal_symbol(const Token *literal, int level) {
        SymbolTypes type;
        ISymbolInfo *info = nullptr;

        switch (literal->type) {
            case TokenType::DEC_INTEGER: {
                long long val = std::stoll(literal->lexeme, nullptr, 0);
                type = SymbolTypes::LITERAL_INTEGER;
                info = new LiteralInfo(type, val);
                break;
            }
            default:
                type = SymbolTypes::NONE;
                info = nullptr;
                break;
        }

        return new SymbolEntry(literal->lexeme, type, info, level);
    }

    static inline SymbolEntry *create_variable_symbol(const std::string &name, Type *var_type, int level) {
        return new SymbolEntry(name, SymbolTypes::SYMBOL_IDENTIFIER, new VariableInfo(name, var_type), level);
    }

    struct SymbolTable {
        std::unordered_map<std::string, SymbolEntry*> table;
        std::unordered_map<int, std::vector<std::string>> level_table;
        int current_level;

        SymbolTable() : table(), level_table(), current_level(0) {
        }

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

        void insert(SymbolEntry *entry, int level) {
            table[entry->name] = entry;
            level_table[level].push_back(entry->name);
        }

        void enter_level() {
            current_level++;
        }

        void exit_level() {
            auto it = level_table.find(current_level);
            if (it != level_table.end()) {
                for (const std::string &name : it->second) {
                    auto entry_it = table.find(name);
                    if (entry_it != table.end()) {
                        delete entry_it->second;
                        table.erase(entry_it);
                    }
                }
                level_table.erase(it);
            }
            current_level--;
        }

        SymbolEntry *lookup_or_insert_literal(const Token *literal) {
            SymbolEntry *entry = lookup(literal->lexeme);
            if (entry == nullptr) {
                entry = create_literal_symbol(literal, current_level);
                insert(entry, 0);
            }
            return entry;
        }

        SymbolEntry *lookup_or_insert_variable(std::string identifier, Type *var_type) {
            SymbolEntry *entry = lookup(identifier);
            if (entry == nullptr) {
                entry = create_variable_symbol(identifier, var_type, current_level);
                insert(entry, current_level);
            }
            return entry;
        }
    };
}