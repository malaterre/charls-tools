#pragma once

#include <string>
#include <vector>

namespace jlst {
std::string get_crc32(std::vector<uint8_t> const& buffer);
} // end namespace jlst
