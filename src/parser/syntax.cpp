#include "parser/syntax.h"

namespace rain {
    const IExprAST *AbstractionNode::body_expr() const {
        return std::get<7>(this->children());
    }

    std::vector<const IExprAST *> ApplicationNode::argument_expr() const {
        auto args = std::get<2>(this->children())->children();
        if (args.empty()) {
            return {};
        }
        std::vector<const IExprAST *> exprs;
        for (auto *arg_connect : args) {
            exprs.push_back(std::get<1>(arg_connect->children()));
        }
        return exprs;
    }
}