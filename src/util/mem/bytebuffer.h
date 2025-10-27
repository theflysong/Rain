#pragma once

#include <cstddef>
#include <stdexcept>
#include <iostream>

namespace rain {
    class bytebuffer {
    private:
        size_t size;
        char *buffer;
        size_t ptr = 0;
    public:
        bytebuffer(size_t size, char *buffer)
            : size(size), buffer(buffer)
        {
        }

        bytebuffer(bytebuffer &&bytebuf)
            : size(bytebuf.size), buffer(bytebuf.buffer), ptr(bytebuf.ptr)
        {
            bytebuf.buffer = nullptr;
        }

        bytebuffer(const std::string &str)
            : size(str.size()), buffer(new char[str.size()])
        {
        }

        ~bytebuffer() {
            if (buffer != nullptr)
                delete[] buffer;
        }

        [[nodiscard]] bool valid_ptr(size_t ptr) const {
            return 0 <= ptr && ptr < size;
        }

        [[nodiscard]] char peer() const {
            if (! valid_ptr(ptr)) {
                throw std::out_of_range("out of range");
            }
            return buffer[ptr];
        }

        char pick() {
            char ch = peer();
            ptr += 1;
            return ch;
        }

        bool rewind(int decreasement) {
            if (! valid_ptr(ptr - decreasement)) {
                return false;
            }

            ptr -= decreasement;
            return true;
        }

        bool ahead(int increasement) {
            if (! valid_ptr(ptr + increasement)) {
                return false;
            }

            ptr += increasement;
            return true;
        }
    };
}