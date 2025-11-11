#pragma once

#include "util/util.h"
#include "parser/ast.h"
#include "codegen/codegen.h"

namespace rain {
    class CompDAG;

    enum class DAGNodeTypes {
        SYMBOL = 0,
        ADD, SUB, MUL, DIV, MOD,
        AND, OR, NOT, XOR, NEG,
        EQ, NEQ, LT, GT, LEQ, GEQ,
        LSH, RSH
    };

    class CompNode {
    protected:
        const CompDAG &dag;
    public:
        std::vector<CompNode*> priors;
        std::vector<CompNode*> successors;
        DAGNodeTypes type;

        CompNode(const CompDAG &dag, DAGNodeTypes type, std::initializer_list<CompNode *> priors) : 
            CompNode(dag, type, std::vector<CompNode*>(priors))
        {
        }

        CompNode(const CompDAG &dag, DAGNodeTypes type, std::vector<CompNode *> &&priors) : 
            dag(dag), type(type), priors(std::move(priors))
        {
            for (auto p : this->priors) {
                p->successors.push_back(this);
            }
        }

        virtual ~CompNode() = default;
    };

    class SymbolNode : public CompNode {
    public:
        const SymbolEntry * _symbol;
        SymbolNode(const CompDAG &dag, const SymbolEntry *symbol) : 
            CompNode(dag, DAGNodeTypes::SYMBOL, {}), _symbol(symbol)
        {
        }
    };

    class CompDAG {
    public:
        std::vector<CompNode*> nodes;
        std::unordered_map<std::string, SymbolNode*> sym_map;

        CompDAG() {
            nodes.reserve(1000);
        }

        CompDAG(const CompDAG &&other) 
            : nodes(std::move(other.nodes))
        {
        }

        ~CompDAG() {
            for (auto node : nodes) {
                delete node;
            }
            nodes.clear();
        }

        void add_node(CompNode*node) {
            nodes.push_back(node);
        }

        void add_node(SymbolNode *node) {
            sym_map[node->_symbol->name] = node;
            nodes.push_back(node);
        }

        SymbolNode *get_sym(std::string name) {
            auto it = sym_map.find(name);
            if (it != sym_map.end()) {
                return it->second;
            }
            return nullptr;
        }
    };
};