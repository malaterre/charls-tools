// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause

#include "source.h"

#include <stdexcept>

#include <cstring>


namespace jlst {

source::source() : stream_(stdin)
{
}

source::source(std::string const& filename)
{
    stream_ = fopen(filename.c_str(), "rb");
    if (!stream_)
        throw std::invalid_argument("bogus filename");
    filename_ = filename;
}

source::~source()
{
    if (!filename_.empty())
        fclose(stream_);
}

int source::peek()
{
    int c;

    c = fgetc(stream_);
    ungetc(c, stream_);

    return c;
}

void source::rewind()
{
    std::rewind(stream_);
}

size_t source::read(void* ptr, size_t n)
{
    return std::fread(ptr, 1, n, stream_);
}

std::string source::getline()
{
    char line[256];
    std::fgets(line, sizeof(line), stream_);
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n')
    {
        line[--len] = '\0';
    }
    return line;
}

std::vector<uint8_t> source::read_bytes()
{
    fseek(stream_, 0, SEEK_END);
    size_t byte_count_file = ftell(stream_);
    rewind();
    std::vector<uint8_t> buffer(byte_count_file);
    read(buffer.data(), buffer.size());
    return buffer;
}

} // end namespace jlst
