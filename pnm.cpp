// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "pnm.h"

#include "dest.h"
#include "factory.h"
#include "image.h"
#include "source.h"
#include "utils.h"

#include <limits>
#include <sstream>
#include <vector>

namespace jlst {
bool pnm::handle_type(std::string const& type) const
{
    return type == "pgm" || type == "ppm";
}
bool pnm::detect(source& s, image_info const&) const
{
    const int c = s.peek();
    return c == 'P';
}

namespace {
// Return 12 for input `4095`
constexpr std::uint32_t log2(const std::uint32_t n) noexcept
{
    std::uint32_t x{};
    while (n > (1u << x))
    {
        ++x;
    }
    return x;
}
static inline std::string& pnm_trim_comment(std::string& s) noexcept
{
    s.erase(s.begin()); // remove leading '#'
    // no need to remove the leading/trailing whitespaces at this point.
    return s;
}
// https://stackoverflow.com/questions/7677158/how-to-detect-negative-numbers-as-parsing-errors-when-reading-unsigned-integers
static inline std::uint32_t parse_u32(std::stringstream& ss)
{
    // use a signed type:
    long long ll;
    // `-0` is read as `0` which is acceptable for our use-case
    if (ss >> ll)
    {
        if (ll >= 0 && ll <= std::numeric_limits<uint32_t>::max())
            return static_cast<std::uint32_t>(ll);
    }
    throw std::invalid_argument("parse_u32");
}
} // namespace

void pnm::read_info(source& fs, image& i) const
{
    auto& ii = i.get_image_info();
    std::string str;
    str = fs.getline();
    if (str != "P5" && str != "P6")
    {
        throw std::invalid_argument(str);
    }
    // component count:
    if (str[1] == '5')
    {
        ii.frame_info().component_count = 1;
        ii.interleave_mode() = charls::interleave_mode::none;
    }
    else if (str[1] == '6')
    {
        ii.frame_info().component_count = 3;
        ii.interleave_mode() = charls::interleave_mode::sample;
    }
    // multi-line comment:
    std::string comment;
    while (fs.peek() == '#')
    {
        str = fs.getline();
        if (!comment.empty())
            comment += "\n"; // FIXME: UNIX style ?
        comment += pnm_trim_comment(str);
    }
    ii.comment() = comment;
    // width / height:
    str = fs.getline();
    {
        std::stringstream ss(str);
        auto w = parse_u32(ss);
        auto h = parse_u32(ss);
        ii.frame_info().width = w;
        ii.frame_info().height = h;
    }
    // bits per sample:
    str = fs.getline();
    {
        std::stringstream ss(str);
        auto bps = parse_u32(ss);
        ii.frame_info().bits_per_sample = log2(bps);
    }
    // now is a good time to compute stride:
    auto const bytes_per_sample{(ii.frame_info().bits_per_sample + 7) / 8};
    i.get_image_data().stride() = ii.frame_info().width * bytes_per_sample * ii.frame_info().component_count;
}

void pnm::read_data(source& fs, image& img) const
{
    auto& id = img.get_image_data();
    auto& pd = id.pixel_data();
    uint8_t* buf8 = pd.data();
    auto len = pd.size();
    fs.read(buf8, len);
    auto& ii = img.get_image_info();
    if (ii.frame_info().bits_per_sample > 8)
    {
        for (size_t i{}; i < len - 1; i += 2)
        {
            std::swap(buf8[i], buf8[i + 1]);
        }
    }
}

void pnm::write_info(dest& d, const image& i, const jls_options&) const
{
    std::stringstream fs;
    auto& ii = i.get_image_info();
    if (ii.frame_info().component_count == 1)
        fs << "P5\n";
    else
        fs << "P6\n";
    // write comment if any:
    auto& comment = ii.comment();
    if (!comment.empty())
    {
        std::stringstream ss(comment);
        std::string com;
        while (ss >> com)
        {
            fs << '#' << com << '\n';
        }
    }

    fs << ii.frame_info().width << " " << ii.frame_info().height << '\n';
    fs << ((1 << ii.frame_info().bits_per_sample) - 1) << '\n';
    const std::string s = fs.str();
    d.write(s.c_str(), s.size());
}

void pnm::write_data(dest& fs, const image& img, jls_options const&) const
{
    auto& ii = img.get_image_info();
    auto& id = img.get_image_data();
    auto& pd = id.pixel_data();

    auto len = pd.size();
    std::vector<unsigned char> buf8(pd);
    auto const bytes_per_sample{(ii.frame_info().bits_per_sample + 7) / 8};
    const size_t stride = ii.frame_info().width * bytes_per_sample * ii.frame_info().component_count;
    if (ii.frame_info().component_count == 3)
    {
        if (ii.interleave_mode() == charls::interleave_mode::none)
        {
            std::vector<unsigned char> copy = utils::planar_to_triplet(buf8, ii.frame_info().width, ii.frame_info().height,
                                                                       ii.frame_info().bits_per_sample, stride);
            buf8.assign(copy.begin(), copy.end());
        }
    }
    if (ii.frame_info().bits_per_sample > 8)
    {
        for (size_t i{}; i < len - 1; i += 2)
        {
            std::swap(buf8[i], buf8[i + 1]);
        }
    }
    fs.write(buf8.data(), buf8.size());
}

format* pnm::clone() const
{
    return new pnm;
}

static const format* get()
{
    static const pnm pnm_;
    return &pnm_;
}

static bool b = factory::instance().register_format(get());
} // namespace jlst
