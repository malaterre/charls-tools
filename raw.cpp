// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "raw.h"
#include "dest.h"
#include "factory.h"
#include "image.h"
#include "source.h"

#include <cassert>
#include <charls/charls.h>

namespace jlst {
bool raw::handle_type(std::string const& type) const
{
    return type == "raw";
}
static charls::frame_info compute_info(size_t const byte_count_file, image_info const& ii)
{
    auto fi = ii.frame_info();
    assert(fi.width != 0 && fi.height != 0);
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
    auto const bytes_per_sample{(i.bits_per_sample + 7) / 8};
    const size_t len = i.width * i.height * bytes_per_sample * i.component_count;
    return len;
}

bool raw::detect(source& s, image_info const& ii) const
{
    auto& fi = ii.frame_info();
    if (fi.width == 0 || fi.height == 0)
        return false;
    const auto byte_count_file = s.size();

    auto info = compute_info(byte_count_file, ii);
    return byte_count_file == compute_len(info);
}

void raw::read_info(source& s, image& i) const
{
    const auto byte_count_file = s.size();
    auto& ii = i.get_image_info();
    auto& fi = ii.frame_info();
    if (fi.width == 0 || fi.height == 0)
        throw std::invalid_argument("Missing size");
    ii.frame_info() = compute_info(byte_count_file, ii);

    if (ii.frame_info().component_count == 3)
    {
        ii.interleave_mode() = charls::interleave_mode::sample;
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

void raw::write_info(dest&, const image&, const jls_options&) const
{
}

void raw::write_data(dest& fs, const image& img, const jls_options&) const
{
    auto& ii = img.get_image_info();
    auto& id = img.get_image_data();
    auto& pd = id.pixel_data();
    auto len = pd.size();
    std::vector<unsigned char> buf8(pd);
    if (ii.frame_info().bits_per_sample > 8)
    {
        for (size_t i{}; i < len - 1; i += 2)
        {
            std::swap(buf8[i], buf8[i + 1]);
        }
    }
    fs.write(buf8.data(), buf8.size());
}

format* raw::clone() const
{
    return new raw;
}

static const format* get()
{
    static const raw raw_;
    return &raw_;
}
// set priority to 0 so that `raw` is always tested last
static bool b = factory::instance().registerFormat(get(), 0);
} // namespace jlst
