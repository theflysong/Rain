#include "parser/ast.h"

namespace rain {
    mem::Pool<IASTNode> IASTNode::pool = mem::Pool<IASTNode>(1000);
}