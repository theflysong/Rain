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

        [[nodiscard]] char peer(int offset = 0) const {
            if (! valid_ptr(ptr + offset)) {
                return '\0';
            }
            return buffer[ptr + offset];
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
                if (ptr + increasement >= size) {
                    ptr = size;
                    return true;
                }
                return false;
            }

            ptr += increasement;
            return true;
        }

        size_t pos() const {
            return ptr;
        }

        std::string slice(size_t start, size_t end = INT_MAX) const {
            end = (end == INT_MAX) ? ptr : end;
            if (end < start) {
                throw std::invalid_argument("end position < start position");
            }
            char data[end - start + 1];
            memcpy(data, buffer + start, end - start);
            data[end - start] = 0;
            return std::string(data);
        }
    };
}