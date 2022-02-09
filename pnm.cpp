// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "pnm.h"
#include "utils.h"

#include <fstream>
#include <limits>
#include <sstream>
#include <vector>

namespace jlst {
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
// bool pnm::detect(const cjpls_options& options)
bool pnm::detect(source& s)
{
#if 0
    assert(0);
    const char* filename; // = options.input.c_str();
    std::ifstream tmp(filename, std::ios::binary);
    std::string str;
    tmp >> str;
    return str[0] == 'P';
#else
    int c = s.peek();
    return c == 'P';
#endif
}
bool pnm::detect2(const djpls_options&)
{
    return true;
}

// void pnm::open(const char* filename, bool b)
//{
//    if (b)
//        fs.open(filename, std::ios::binary | std::ios::out);
//    else
//        fs.open(filename, std::ios::binary | std::ios::in);
//}
// void pnm::close()
//{
//    fs.close();
//}

static std::string& pnm_trim_comment(std::string& s)
{
    s.erase(s.begin()); // remove leading '#'
    while (s.compare(0, 1, " ") == 0)
        s.erase(s.begin()); // leading whitespaces
    while (s.size() > 0 && s.compare(s.size() - 1, 1, " ") == 0)
        s.erase(s.end() - 1); // trailing whitespaces
    return s;
}

void pnm::read_info(source& fs)
{
    std::string str;
    str = fs.getline();
    if (str != "P5" && str != "P6")
    {
        throw std::invalid_argument(str);
    }
    // component count:
    if (str[1] == '5')
    {
        mode_ = mode::none;
        info_.component_count = 1;
    }
    else if (str[1] == '6')
    {
        mode_ = mode::sample;
        info_.component_count = 3;
    }
    // skip comment
    while (fs.peek() == '#')
    {
        str = fs.getline();
        if (!comment.empty())
            comment += "\n";
        comment += pnm_trim_comment(str);
    }
    // width / height:
    str = fs.getline();
    {
        std::stringstream ss(str);
        int w;
        int h;
        ss >> w;
        if (w <= 0)
            throw std::invalid_argument(str);
        ss >> h;
        if (h <= 0)
            throw std::invalid_argument(str);

        info_.width = w;
        info_.height = h;
    }
    // bits per sample:
    str = fs.getline();
    {
        std::stringstream ss(str);
        long long i;
        ss >> i;
        if (i <= 0 || i >= std::numeric_limits<uint32_t>::max())
            throw std::invalid_argument(str);

        info_.bits_per_sample = log2(i);
    }
    // now is a good time to compute stride:
    auto const bytes_per_sample{(info_.bits_per_sample + 7) / 8};
    stride_ = info_.width * bytes_per_sample * info_.component_count;
}

void pnm::read_data(source& fs, void* buffer, size_t len)
{
    char* buf8 = static_cast<char*>(buffer);
    fs.read(buf8, len);
    if (info_.bits_per_sample > 8)
    {
        for (size_t i{}; i < len - 1; i += 2)
        {
            std::swap(buf8[i], buf8[i + 1]);
        }
    }
}

void pnm::write_info(dest& d)
{
    std::stringstream fs;
    if (info_.component_count == 1)
        fs << "P5\n";
    else
        fs << "P6\n";
    fs << info_.width << " " << info_.height << '\n';
    fs << ((1 << info_.bits_per_sample) - 1) << '\n';
    std::string s = fs.str();
    d.write(s.c_str(), s.size());
}

void pnm::write_data(dest& fs, const void* buffer, size_t len)
{
    const unsigned char* cbuffer = static_cast<const unsigned char*>(buffer);
    std::vector<unsigned char> buf8;
    buf8.assign(cbuffer, cbuffer + len);
    const size_t stride = info_.width * (info_.bits_per_sample / 8) * info_.component_count;
    if (info_.component_count == 3)
    {
        if (mode_ == charls::interleave_mode::none)
        {
            std::vector<unsigned char> copy =
                utils::planar_to_triplet(buf8, info_.width, info_.height, info_.bits_per_sample, stride);
            buf8.assign(copy.begin(), copy.end());
        }
    }
    if (info_.bits_per_sample > 8)
    {
        for (size_t i{}; i < len - 1; i += 2)
        {
            std::swap(buf8[i], buf8[i + 1]);
        }
    }
    fs.write(reinterpret_cast<char*>(buf8.data()), buf8.size());
}

format& pnm::get()
{
    static pnm pnm_;
    return pnm_;
}
} // namespace jlst
