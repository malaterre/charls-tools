// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "image.h"

#include "utils.h"

#include <cassert>
#include <cstring>   // memset
#include <stdexcept> // for invalid_argument

namespace jlst {

bool image_info::invalid()
{
    return frame_info_.width == 0;
}
inline bool operator==(const charls::frame_info& lhs, const charls::frame_info& rhs)
{
    return lhs.width == rhs.width && lhs.height == rhs.height && lhs.bits_per_sample == rhs.bits_per_sample &&
           lhs.component_count == rhs.component_count;
}
bool image_info::operator==(const image_info& other) const
{
    const bool b1 = frame_info_ == other.frame_info_;
    const bool b2 = interleave_mode_ == other.interleave_mode_;
    return b1 && b2;
}
void image_data::append(image_data const& id)
{
    pixel_data_.insert(pixel_data_.end(), id.pixel_data_.begin(), id.pixel_data_.end());
}

void image::append(image const& other)
{
    if (image_info_.invalid())
    {
        image_info_ = other.image_info_;
        image_data_ = other.image_data_;
    }
    else
    {
        if (image_info_ == other.image_info_)
        {
            image_data_.append(other.image_data_);
        }
    }
}

std::vector<uint8_t> image::transform(charls::interleave_mode const& interleave_mode) const
{
    auto& frame_info = get_image_info().frame_info();
    if (frame_info.component_count == 1 || get_image_info().interleave_mode() == interleave_mode)
        return get_image_data().pixel_data();
    if (frame_info.component_count == 3)
    {
        if (interleave_mode == charls::interleave_mode::none)
        {
            assert(get_image_info().interleave_mode() == charls::interleave_mode::sample);
            return utils::triplet_to_planar(get_image_data().pixel_data(), frame_info.width, frame_info.height,
                                            frame_info.bits_per_sample, get_image_data().stride());
        }
        else if (interleave_mode == charls::interleave_mode::line)
        {
            if (get_image_info().interleave_mode() == charls::interleave_mode::sample)
                return get_image_data().pixel_data();
            assert(get_image_info().interleave_mode() == charls::interleave_mode::none);
            return utils::planar_to_triplet(get_image_data().pixel_data(), frame_info.width, frame_info.height,
                                            frame_info.bits_per_sample, get_image_data().stride());
        }
        else if (interleave_mode == charls::interleave_mode::sample)
        {
            assert(get_image_info().interleave_mode() == charls::interleave_mode::none);
            return utils::planar_to_triplet(get_image_data().pixel_data(), frame_info.width, frame_info.height,
                                            frame_info.bits_per_sample, get_image_data().stride());
        }
    }
    throw std::invalid_argument("invalid transform request");
}

std::vector<uint8_t> image::crop(uint32_t X, uint32_t Y, uint32_t width, uint32_t height)
{
    assert(get_image_data().stride() == 0);
    auto& frame_info = get_image_info().frame_info();
    auto& inbuffer = get_image_data().pixel_data();
    std::vector<uint8_t> out;
    size_t nbytes = frame_info.component_count * ((frame_info.bits_per_sample + 7) / 8);
    out.resize(width * height * nbytes);
    auto inwidth = frame_info.width;
    for (uint32_t y{}; y != frame_info.height; ++y)
    {
        for (uint32_t x{}; x != frame_info.width; ++x)
        {
            if ((x >= X && x < X + width) && (y >= Y && y < Y + height))
            {
                std::memcpy(&out[(y - Y) * width * nbytes + (x - X) * nbytes], &inbuffer[y * inwidth * nbytes + x * nbytes],
                            nbytes);
            }
        }
    }
    return out;
}
std::vector<uint8_t> image::flip(bool vertical)
{
    assert(get_image_data().stride() == 0);
    auto& frame_info = get_image_info().frame_info();
    auto& inbuffer = get_image_data().pixel_data();
    std::vector<uint8_t> out;
    out.resize(inbuffer.size());
    size_t nbytes = frame_info.component_count * ((frame_info.bits_per_sample + 7) / 8);
    auto inheigth = frame_info.height;
    auto inwidth = frame_info.width;
    if (vertical)
    {
        for (uint32_t y{}; y != frame_info.height; ++y)
        {
            std::memcpy(&out[y * inwidth * nbytes], &inbuffer[(inheigth - y - 1) * inwidth * nbytes], nbytes * inwidth);
        }
    }
    else
    {
        // horizontal
        for (uint32_t y{}; y != frame_info.height; ++y)
        {
            for (uint32_t x{}; x != frame_info.width; ++x)
            {
                std::memcpy(&out[y * inwidth * nbytes + x * nbytes],
                            &inbuffer[y * inwidth * nbytes + (inwidth - x - 1) * nbytes], nbytes);
            }
        }
    }
    return out;
}
std::vector<uint8_t> image::rotate(int degree)
{
    assert(degree == 90 || degree == 180 || degree == 270);
    assert(get_image_data().stride() == 0);
    auto& frame_info = get_image_info().frame_info();
    auto& inbuffer = get_image_data().pixel_data();
    std::vector<uint8_t> out;
    out.resize(inbuffer.size());
    size_t nbytes = frame_info.component_count * ((frame_info.bits_per_sample + 7) / 8);
    if (degree == 90)
    {
        auto oldheight = frame_info.height;
        auto oldwidth = frame_info.width;
        auto newheight = frame_info.width;
        auto newwidth = frame_info.height;

        for (uint32_t y{}; y != newheight; ++y)
        {
            for (uint32_t x{}; x != newwidth; ++x)
            {
                std::memcpy(&out[y * newwidth * nbytes + x * nbytes],
                            &inbuffer[(oldheight - x - 1) * oldwidth * nbytes + y * nbytes], nbytes);
            }
        }
    }
    else if (degree == 180)
    {
        auto oldheight = frame_info.height;
        auto oldwidth = frame_info.width;
        for (uint32_t y{}; y != oldheight; ++y)
        {
            for (uint32_t x{}; x != oldwidth; ++x)
            {
                std::memcpy(&out[y * oldwidth * nbytes + x * nbytes],
                            &inbuffer[(oldheight - y - 1) * oldwidth * nbytes + (oldwidth - x - 1) * nbytes], nbytes);
            }
        }
    }
    else if (degree == 270)
    {
        auto oldheight = frame_info.height;
        auto oldwidth = frame_info.width;
        auto newheight = frame_info.width;
        auto newwidth = frame_info.height;

        for (uint32_t y{}; y != newheight; ++y)
        {
            for (uint32_t x{}; x != newwidth; ++x)
            {
                std::memcpy(&out[y * newwidth * nbytes + x * nbytes],
                            &inbuffer[x * oldwidth * nbytes + (oldwidth - y - 1) * nbytes], nbytes);
            }
        }
    }
    return out;
}
std::vector<uint8_t> image::transpose()
{
    assert(get_image_data().stride() == 0);
    auto& frame_info = get_image_info().frame_info();
    auto& inbuffer = get_image_data().pixel_data();
    std::vector<uint8_t> out;
    out.resize(inbuffer.size());
    size_t nbytes = frame_info.component_count * ((frame_info.bits_per_sample + 7) / 8);

    auto oldheight = frame_info.height;
    auto oldwidth = frame_info.width;
    auto newheight = frame_info.width;
    auto newwidth = frame_info.height;

    for (uint32_t y{}; y != newheight; ++y)
    {
        for (uint32_t x{}; x != newwidth; ++x)
        {
            std::memcpy(&out[y * newwidth * nbytes + x * nbytes], &inbuffer[x * oldwidth * nbytes + y * nbytes], nbytes);
        }
    }
    return out;
}
std::vector<uint8_t> image::transverse()
{
    assert(get_image_data().stride() == 0);
    auto& frame_info = get_image_info().frame_info();
    auto& inbuffer = get_image_data().pixel_data();
    std::vector<uint8_t> out;
    out.resize(inbuffer.size());
    size_t nbytes = frame_info.component_count * ((frame_info.bits_per_sample + 7) / 8);

    auto oldheight = frame_info.height;
    auto oldwidth = frame_info.width;
    auto newheight = frame_info.width;
    auto newwidth = frame_info.height;

    for (uint32_t y{}; y != newheight; ++y)
    {
        for (uint32_t x{}; x != newwidth; ++x)
        {
            std::memcpy(&out[y * newwidth * nbytes + x * nbytes],
                        &inbuffer[(oldheight - x - 1) * oldwidth * nbytes + (oldwidth - y - 1) * nbytes], nbytes);
        }
    }
    return out;
}
std::vector<uint8_t> image::wipe(uint32_t X, uint32_t Y, uint32_t width, uint32_t height)
{
    assert(get_image_data().stride() == 0);
    auto& frame_info = get_image_info().frame_info();
    auto& inbuffer = get_image_data().pixel_data();
    std::vector<uint8_t> out;
    out.resize(inbuffer.size());
    size_t nbytes = frame_info.component_count * ((frame_info.bits_per_sample + 7) / 8);
    auto inwidth = frame_info.width;
    for (uint32_t y{}; y != frame_info.height; ++y)
    {
        for (uint32_t x{}; x != frame_info.width; ++x)
        {
            if ((x >= X && x < X + width) && (y >= Y && y < Y + height))
            {
                std::memset(&out[y * inwidth * nbytes + x * nbytes], 0, nbytes);
            }
            else
            {
                std::memcpy(&out[y * inwidth * nbytes + x * nbytes], &inbuffer[y * inwidth * nbytes + x * nbytes], nbytes);
            }
        }
    }
    return out;
}

} // namespace jlst
