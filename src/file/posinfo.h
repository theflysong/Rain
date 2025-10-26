#pragma once

#include "util/util.h"

namespace rain {
    struct PosInfo {
        static mem::Pool<PosInfo> pool;

        std::string path;
        int line;
        int column;

        PosInfo(std::string path, int line, int column)
            : path(path), line(line), column(column)
        {
            pool.mark(this);
        }
    };
}