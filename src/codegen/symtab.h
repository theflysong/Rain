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
    
    struct SymbolEntry {
        std::string name;
        SymbolTypes type;
        int level;
        // Move variable information here for SYMBOL_IDENTIFIER
        long long int_value;
        Type *var_type;

        SymbolEntry(std::string name, SymbolTypes type, int level, Type *var_type = nullptr, long long int_value = 0)
            : name(std::move(name)), type(type), level(level), var_type(var_type), int_value(int_value)
        {}

        ~SymbolEntry() {
            // var_type is owned by the compiler/type system; do not delete here
        }
    };
    
    static inline SymbolEntry *create_literal_symbol(const Token *literal, int level) {
        SymbolTypes type;
        long long val;

        switch (literal->type) {
            case TokenType::DEC_INTEGER: {
                long long val = std::stoll(literal->lexeme, nullptr, 0);
                type = SymbolTypes::LITERAL_INTEGER;
            }
            default:
                type = SymbolTypes::NONE;
                val = 0;
        }

        return new SymbolEntry(literal->lexeme, type, level, &Type::NAT_TYPE, val);
    }

    static inline SymbolEntry *create_variable_symbol(const std::string &name, Type *var_type, int level) {
        return new SymbolEntry(name, SymbolTypes::SYMBOL_IDENTIFIER, level, var_type, 0);
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