#pragma once

#include "util/util.h"
#include "parser/ast.h"

namespace rain {
    class CompDAG;

    enum class DAGNodeTypes {
        LITERAL = 0, VARIABLE,
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

    class LiteralNode : public CompNode {
    public:
        //TODO: 将其改为Symbol
        const Token *_value;
        LiteralNode(const CompDAG &dag, Token *value) : 
            CompNode(dag, DAGNodeTypes::LITERAL, {}), _value(value)
        {
        }
    };

    class VariableNode : public CompNode {
    public:
        //TODO: 将其改为Symbol
        std::string name;
        VariableNode(const CompDAG &dag, const std::string &name) : 
            CompNode(dag, DAGNodeTypes::VARIABLE, {}), name(name)
        {
        }
    };

    class CompDAG {
    public:
        std::vector<CompNode*> nodes;

        CompDAG() 
        {
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

        void add_node(CompNode *node) {
            nodes.push_back(node);
        }
    };
};