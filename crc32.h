// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace jlst {
class crc32
{
public:
    static std::string compute(std::vector<uint8_t> const& buffer);
};
} // end namespace jlst
