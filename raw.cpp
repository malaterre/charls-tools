// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "raw.h"

#include <fstream>
#include <sstream>

namespace jlst {
static raw::info compute_info(size_t const byte_count_file, cjpls_options const& options)
{
    auto i = options.frame_info;
    const size_t div = byte_count_file % (i.width * i.height);
    // be nice with user, and compute default bits_per_sample / component_count if not specified:
    if (div == 0)
    {
        // mult is either 1, 2, 3 or 6:
        const size_t mult = byte_count_file / (i.width * i.height);
        if (i.bits_per_sample == 0)
        {
            i.bits_per_sample = mult % 2 == 0 ? 16 : 8;
        }
        if (i.component_count == 0)
        {
            i.component_count = mult % 3 == 0 ? 3 : 1;
        }
    }

    return i;
}

static size_t compute_len(raw::info const& i)
{
    size_t bits_per_sample = 0;
    if (i.bits_per_sample <= 8)
    {
        bits_per_sample = 8;
    }
    else if (i.bits_per_sample <= 16)
    {
        bits_per_sample = 16;
    }
    const size_t len = i.width * i.height * (bits_per_sample / 8) * i.component_count;
    return len;
}

bool raw::detect(cjpls_options const& options)
{
    info const inf = options.frame_info;
    if (inf.width == 0 || inf.height == 0)
        return false;
    const char* filename = options.input.c_str();
    std::ifstream tmp(filename, std::ios::binary);
    tmp.seekg(0, std::ios::end);
    const auto byte_count_file = static_cast<size_t>(tmp.tellg());

    info_ = compute_info(byte_count_file, options);
    mode_ = mode::none;
    if (info_.component_count == 3)
    {
        mode_ = options.planar_configuration;
    }
    stride_ = info_.width * (info_.bits_per_sample / 8) * info_.component_count;
    return byte_count_file == compute_len(info_);
}

bool raw::detect2(djpls_options const&)
{
    return false;
}

void raw::open(const char* filename, bool b)
{
    ifs.open(filename, std::ios::binary);
}
void raw::close()
{
    ifs.close();
}

void raw::read_info()
{
}

void raw::read_data(void* buffer, size_t len)
{
    ifs.read(static_cast<char*>(buffer), len);
}

void raw::write_info()
{
}

void raw::write_data(void* buffer, size_t len)
{
    //    ifs.write(static_cast<char*>(buffer), len);
}

format& raw::get()
{
    static raw raw1;
    return raw1;
}
} // namespace jlst
