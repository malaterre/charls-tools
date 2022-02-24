// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause

#include "dest.h"

#include <stdexcept>

namespace jlst {

dest::dest() : stream_(stdout)
{
}

dest::dest(std::string const& filename)
{
    stream_ = std::fopen(filename.c_str(), "wb");
    if (!stream_)
        throw std::invalid_argument("bogus filename");
    filename_ = filename;
}

dest::~dest()
{
    if (!filename_.empty())
        std::fclose(stream_);
}

size_t dest::write(const void* ptr, size_t n)
{
    return std::fwrite(ptr, 1, n, stream_);
}

} // end namespace jlst
