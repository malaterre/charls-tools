// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cassert>
#include <cstdlib> // EXIT_SUCCESS
#include <fstream>
#include <iostream>
#include <vector>

#include <charls/charls.h>


namespace jlst {

struct utils final
{
    static std::vector<uint8_t> triplet_to_planar(const std::vector<uint8_t>& buffer, const uint16_t width,
                                                  const uint16_t height, const uint8_t bits_per_sample, const size_t stride);

    static std::vector<uint8_t> planar_to_triplet(const std::vector<uint8_t>& buffer, const uint16_t width,
                                                  const uint16_t height, const uint8_t bits_per_sample, const size_t stride);

    static void save_buffer_to_file(const void* buffer, const size_t buffer_size, const char* filename);
};

} // namespace jlst
