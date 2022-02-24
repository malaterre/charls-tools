// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "crc32.h"

#include <boost/crc.hpp>
#include <cstdio> // std::sprintf

namespace jlst {
std::string crc32::compute(std::vector<uint8_t> const& buffer)
{
    boost::crc_32_type result;
    result.process_bytes(buffer.data(), buffer.size());
    uint32_t checksum = result.checksum();
    char crc32[16];
    std::sprintf(crc32, "%8x", checksum);
    return crc32;
}
} // namespace jlst
