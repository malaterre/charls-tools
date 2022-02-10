// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "pnm.h"
#include "image.h"
#include "utils.h"

#include <fstream>
#include <limits>
#include <sstream>
#include <vector>

namespace jlst {
bool pnm::detect(source& s) const
{
    const int c = s.peek();
    return c == 'P';
}
bool pnm::detect2(const djpls_options&) const
{
    return true;
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
static std::string& pnm_trim_comment(std::string& s) noexcept
{
    s.erase(s.begin()); // remove leading '#'
    while (s.compare(0, 1, " ") == 0)
        s.erase(s.begin()); // leading whitespaces
    while (s.size() > 0 && s.compare(s.size() - 1, 1, " ") == 0)
        s.erase(s.end() - 1); // trailing whitespaces
    return s;
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
    // comment
    std::string comment;
    while (fs.peek() == '#')
    {
        str = fs.getline();
        if (!comment.empty())
            comment += "\n";
        comment += pnm_trim_comment(str);
    }
    ii.comment() = comment;
    // width / height:
    str = fs.getline();
    {
        std::stringstream ss(str);
        long long w;
        long long h;
        ss >> w;
        if (w <= 0 || w >= std::numeric_limits<uint32_t>::max())
            throw std::invalid_argument(str);
        ss >> h;
        if (h <= 0 || h >= std::numeric_limits<uint32_t>::max())
            throw std::invalid_argument(str);

        ii.frame_info().width = w;
        ii.frame_info().height = h;
    }
    // bits per sample:
    str = fs.getline();
    {
        std::stringstream ss(str);
        long long bps;
        ss >> bps;
        if (bps <= 0 || bps >= std::numeric_limits<uint32_t>::max())
            throw std::invalid_argument(str);

        ii.frame_info().bits_per_sample = log2(bps);
    }
    // now is a good time to compute stride:
    auto const bytes_per_sample{(ii.frame_info().bits_per_sample + 7) / 8};
    i.get_image_data().stride() = ii.frame_info().width * bytes_per_sample * ii.frame_info().component_count;
}

void pnm::read_data(source& fs, image& i) const
{
    auto& id = i.get_image_data();
    auto& pd = id.pixel_data();
    uint8_t* buf8 = pd.data();
    auto len = pd.size();
    fs.read(buf8, len);
    auto& ii = i.get_image_info();
    if (ii.frame_info().bits_per_sample > 8)
    {
        for (size_t i{}; i < len - 1; i += 2)
        {
            std::swap(buf8[i], buf8[i + 1]);
        }
    }
}

void pnm::write_info(dest& d, const image& i) const
{
    std::stringstream fs;
    auto& ii = i.get_image_info();
    if (ii.frame_info().component_count == 1)
        fs << "P5\n";
    else
        fs << "P6\n";
    fs << ii.frame_info().width << " " << ii.frame_info().height << '\n';
    fs << ((1 << ii.frame_info().bits_per_sample) - 1) << '\n';
    const std::string s = fs.str();
    d.write(s.c_str(), s.size());
}

void pnm::write_data(dest& fs, const image& i) const
{
    auto& ii = i.get_image_info();
    auto& id = i.get_image_data();
    auto& pd = id.pixel_data();

    auto len = pd.size();
    std::vector<unsigned char> buf8(pd);
    const size_t stride = ii.frame_info().width * (ii.frame_info().bits_per_sample / 8) * ii.frame_info().component_count;
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
    fs.write(reinterpret_cast<char*>(buf8.data()), buf8.size());
}

const format& pnm::get()
{
    static const pnm pnm_;
    return pnm_;
}
} // namespace jlst
