#include "codegen/type.h"

namespace rain {
    Type Type::NAT_TYPE(TypeKinds::TYPE_NAT);

    bool type_match(Type *a, Type *b) {
        if (a->kind != b->kind) {
            return false;
        }
        
        if (a->kind == TypeKinds::TYPE_NAT) {
            return true;
        }
        
        // Arrow Type
        return type_match(a->from, b->from) && type_match(a->to, b->to);
    }

    Type *Type::apply(Type *arg) const {
        if (this->kind != TypeKinds::TYPE_ARROW) {
            return nullptr;
        }
        if (! type_match(this->from, arg)) {
            return nullptr;
        }
        return this->to;
    }
}