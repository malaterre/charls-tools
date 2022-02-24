// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "utils.h"

#include <cstring>   // std::memcpy
#include <stdexcept> // std::invalid_argument

namespace jlst {

template<typename T>
static std::vector<T> triplet_to_planar_impl(const std::vector<T>& buffer, const size_t width, const size_t height,
                                             const size_t stride)
{
    constexpr size_t bytes_per_rgb_pixel{3};
    std::vector<T> result(bytes_per_rgb_pixel * width * height);
    const size_t byte_count_plane{width * height};

    size_t plane_column{};
    for (size_t line{}; line != height; ++line)
    {
        const auto line_start{line * stride};
        for (size_t pixel{}; pixel != width; ++pixel)
        {
            const auto column{line_start + pixel * bytes_per_rgb_pixel};
            result[plane_column + 0 * byte_count_plane] = buffer[column + 0];
            result[plane_column + 1 * byte_count_plane] = buffer[column + 1];
            result[plane_column + 2 * byte_count_plane] = buffer[column + 2];
            ++plane_column;
        }
    }

    return result;
}

template<typename T>
static std::vector<T> planar_to_triplet_impl(const std::vector<T>& buffer, const size_t width, const size_t height,
                                             const size_t stride)
{
    constexpr size_t bytes_per_rgb_pixel{3};
    std::vector<T> result(bytes_per_rgb_pixel * width * height);
    const size_t byte_count_plane{width * height};

    size_t plane_column{};
    for (size_t line{}; line != height; ++line)
    {
        const auto line_start{line * stride};
        for (size_t pixel{}; pixel != width; ++pixel)
        {
            const auto column{line_start + pixel * bytes_per_rgb_pixel};
            result[column + 0] = buffer[plane_column + 0 * byte_count_plane];
            result[column + 1] = buffer[plane_column + 1 * byte_count_plane];
            result[column + 2] = buffer[plane_column + 2 * byte_count_plane];
            ++plane_column;
        }
    }

    return result;
}

std::vector<uint8_t> utils::triplet_to_planar(const std::vector<uint8_t>& buffer, const uint16_t width,
                                              const uint16_t height, const uint8_t bits_per_sample, const size_t stride)
{
    if (bits_per_sample <= 8)
    {
        return triplet_to_planar_impl(buffer, width, height, stride);
    }
    else if (bits_per_sample <= 16)
    {
        std::vector<uint16_t> buffer16;
        buffer16.resize(buffer.size() / 2);
        std::memcpy(buffer16.data(), buffer.data(), buffer.size());
        std::vector<uint16_t> tmp{triplet_to_planar_impl(buffer16, width, height, stride / 2)};
        std::vector<uint8_t> ret;
        ret.resize(buffer.size());
        std::memcpy(ret.data(), tmp.data(), buffer.size());
        return ret;
    }
    throw std::invalid_argument("triplet_to_planar");
}

std::vector<uint8_t> utils::planar_to_triplet(const std::vector<uint8_t>& buffer, const uint16_t width,
                                              const uint16_t height, const uint8_t bits_per_sample, const size_t stride)
{
    if (bits_per_sample <= 8)
    {
        return planar_to_triplet_impl(buffer, width, height, stride);
    }
    else if (bits_per_sample <= 16)
    {
        std::vector<uint16_t> buffer16;
        buffer16.resize(buffer.size() / 2);
        std::memcpy(buffer16.data(), buffer.data(), buffer.size());
        std::vector<uint16_t> tmp{planar_to_triplet_impl(buffer16, width, height, stride / 2)};
        std::vector<uint8_t> ret;
        ret.resize(buffer.size());
        std::memcpy(ret.data(), tmp.data(), buffer.size());
        return ret;
    }
    throw std::invalid_argument("planar_to_triplet");
}

} // namespace jlst
