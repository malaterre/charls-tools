// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cstddef> // std::size_t
#include <cstdint>
#include <vector>

namespace jlst {
struct utils final
{
    static std::vector<uint8_t> triplet_to_planar(const std::vector<uint8_t>& buffer, const size_t width,
                                                  const size_t height, const uint8_t bits_per_sample, const size_t stride);

    static std::vector<uint8_t> planar_to_triplet(const std::vector<uint8_t>& buffer, const size_t width,
                                                  const size_t height, const uint8_t bits_per_sample, const size_t stride);
};

} // namespace jlst
