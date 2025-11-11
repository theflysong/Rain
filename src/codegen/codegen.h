#pragma once

#include "codegen/symtab.h"

namespace rain {
    struct CodeGenContext {
        SymbolTable &symtab;

        CodeGenContext(SymbolTable &symtab)
            : symtab(symtab)
        {
        }
    };
}