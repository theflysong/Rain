#include "file/helper.h"

#include <cstdio>

using namespace rain;

bytebuffer rain::readall(const char *filepath)
{
    size_t filesz;
    char *buffer;
    FILE *fp = fopen(filepath, "rb");

    if (fp == nullptr) {
        throw std::invalid_argument("file path doesn't exist");
    }

    assert(! fseek(fp, 0, SEEK_END));
    assert(filesz = ftell(fp));
    assert(! fseek(fp, 0, SEEK_SET));
    
    if ((buffer = new char[filesz]) == nullptr) {
        throw std::bad_alloc();
    }

    size_t sz_read = fread(buffer, sizeof(char), filesz, fp);

    assert(sz_read == filesz);
    fclose(fp);

    return std::move(bytebuffer(filesz, buffer));
}