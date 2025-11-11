#pragma once
#include "compdag.h"
#include <fstream>
#include <string>

namespace rain {
    // 将DAG导出为Graphviz dot格式
    class DAGVisualizer {
    public:
        // 输出到指定文件
        static void export_to_dot(const CompDAG &dag, const std::string &filename, CodeGenContext &ctx) {
            std::ofstream ofs(filename);
            if (!ofs.is_open()) return;
            ofs << "digraph CompDAG {\n";
            // 输出所有节点
            for (const auto *node : dag.nodes) {
                ofs << "  \"" << (void*)node << "\" [label=\"" << node_type_to_string(node->type);
                if (node->type == DAGNodeTypes::SYMBOL) {
                    auto sym = static_cast<const SymbolNode*>(node);
                    if (sym == nullptr && sym->_symbol->info == nullptr) {
                        ofs << "\\nnull";
                    } else {
                        ofs << "\\n" << sym->_symbol->info->repr();
                    }
                } 
                ofs << "\"]\n";
            }
            // 输出所有边
            for (const auto *node : dag.nodes) {
                for (const auto *prior : node->priors) {
                    ofs << "  \"" << (void*)prior << "\" -> \"" << (void*)node << "\"\n";
                }
            }
            ofs << "}\n";
            ofs.close();
        }
    private:
        static const char* node_type_to_string(DAGNodeTypes type) {
            switch (type) {
            case DAGNodeTypes::SYMBOL: return "SYMBOL";
            case DAGNodeTypes::ADD: return "+";
            case DAGNodeTypes::SUB: return "-";
            case DAGNodeTypes::MUL: return "*";
            case DAGNodeTypes::DIV: return "/";
            case DAGNodeTypes::MOD: return "%";
            case DAGNodeTypes::AND: return "&";
            case DAGNodeTypes::OR: return "|";
            case DAGNodeTypes::NOT: return "!";
            case DAGNodeTypes::XOR: return "^";
            case DAGNodeTypes::NEG: return "NEG";
            case DAGNodeTypes::EQ: return "==";
            case DAGNodeTypes::NEQ: return "!=";
            case DAGNodeTypes::LT: return "<";
            case DAGNodeTypes::GT: return ">";
            case DAGNodeTypes::LEQ: return "<=";
            case DAGNodeTypes::GEQ: return ">=";
            case DAGNodeTypes::LSH: return "<<";
            case DAGNodeTypes::RSH: return ">>";
            default: return "?";
            }
        }
    };
}
