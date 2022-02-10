// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "raw.h"

#include <fstream>
#include <sstream>

#include <cassert>

namespace jlst {
static charls::frame_info compute_info(size_t const byte_count_file, image_info const& ii)
{
    auto fi = ii.frame_info();
    const size_t div = byte_count_file % (fi.width * fi.height);
    // be nice with user, and compute default bits_per_sample / component_count if not specified:
    if (div == 0)
    {
        // mult is either 1, 2, 3 or 6:
        const size_t mult = byte_count_file / (fi.width * fi.height);
        if (fi.bits_per_sample == 0)
        {
            fi.bits_per_sample = mult % 2 == 0 ? 16 : 8;
        }
        if (fi.component_count == 0)
        {
            fi.component_count = mult % 3 == 0 ? 3 : 1;
        }
    }

    return fi;
}

static size_t compute_len(charls::frame_info const& i)
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

bool raw::detect(image_info const& ii, source& s) const
{
    auto& fi = ii.frame_info();
    if (fi.width == 0 || fi.height == 0)
        return false;
    const auto byte_count_file = s.size();

    auto info = compute_info(byte_count_file, ii);
    auto mode = charls::interleave_mode::none;
    if (info.component_count == 3)
    {
        mode = ii.interleave_mode();
    }
    auto const bytes_per_sample{(info.bits_per_sample + 7) / 8};
    auto stride = info.width * bytes_per_sample * info.component_count;
    return byte_count_file == compute_len(info);
}

bool raw::detect2(djpls_options const&) const
{
    return false;
}

void raw::read_info(source& s, image& i) const
{
    const auto byte_count_file = s.size();
    auto& ii = i.get_image_info();
    ii.frame_info() = compute_info(byte_count_file, ii);

    if (ii.frame_info().component_count == 3)
    {
        ii.interleave_mode() == charls::interleave_mode::sample;
    }

    // now is a good time to compute stride:
    auto const bytes_per_sample{(ii.frame_info().bits_per_sample + 7) / 8};
    i.get_image_data().stride() = ii.frame_info().width * bytes_per_sample * ii.frame_info().component_count;
}

void raw::read_data(source& ifs, image& i) const
{
    auto& pd = i.get_image_data().pixel_data();
    ifs.read(pd.data(), pd.size());
}

void raw::write_info(dest& d, const image& i) const
{
}

void raw::write_data(dest& d, const image& i) const
{
    //    ifs.write(static_cast<char*>(buffer), len);
}

const format& raw::get()
{
    static const raw raw_;
    return raw_;
}
} // namespace jlst
