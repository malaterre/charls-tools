// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause

#include "source.h"

#include <stdexcept>

#include <cassert>
#include <cstring>

namespace jlst {

source::source() : stream_(stdin)
{
}

source::source(std::string const& filename)
{
    stream_ = std::fopen(filename.c_str(), "rb");
    if (!stream_)
        throw std::invalid_argument("bogus filename");
    filename_ = filename;
}

source::~source()
{
    if (!filename_.empty())
        std::fclose(stream_);
}

int source::peek()
{
    const int c = std::fgetc(stream_);
    std::ungetc(c, stream_);

    return c;
}

void source::rewind()
{
    std::rewind(stream_);
}

size_t source::read(void* ptr, size_t n)
{
    const size_t nr = std::fread(ptr, 1, n, stream_);
    assert(n == nr);
    return nr;
}

std::string source::getline()
{
    char line[256];
    std::fgets(line, sizeof(line), stream_);
    size_t len = std::strlen(line);
    if (len > 0 && line[len - 1] == '\n')
    {
        line[--len] = '\0';
    }
    return line;
}

size_t source::size()
{
    std::fseek(stream_, 0, SEEK_END);
    const size_t byte_count_file = std::ftell(stream_);
    rewind();
    return byte_count_file;
}

std::vector<uint8_t> source::read_bytes()
{
    assert(std::ftell(stream_) == 0);
    std::vector<uint8_t> buffer(size());
    read(buffer.data(), buffer.size());
    return buffer;
}

} // end namespace jlst
