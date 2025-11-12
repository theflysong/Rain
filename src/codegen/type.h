#pragma once

#include <string>

namespace rain {
    enum class TypeKinds {
        TYPE_NAT,
        TYPE_ARROW,
    };

    struct Type {
    public:
        TypeKinds kind;
        Type *from;
        Type *to;

        Type(TypeKinds kind) : kind(kind) {}
        Type(Type *from, Type *to) : kind(TypeKinds::TYPE_ARROW), from(from), to(to) {}

        virtual ~Type() {
        }
        
        Type *apply(Type *arg) const;

        std::string repr() const {
            switch (kind) {
                case TypeKinds::TYPE_NAT:
                    return "Nat";
                case TypeKinds::TYPE_ARROW:
                    return "(" + from->repr() + " -> " + to->repr() + ")";
                default:
                    return "<unknown type>";
            }
        }
    };

    bool type_match(Type *a, Type *b);
}