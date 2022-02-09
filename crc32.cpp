#include "crc32.h"

#include <boost/crc.hpp>
#include <cstdint>
#include <string>

namespace jlst {
std::string get_crc32(std::vector<uint8_t> const& buffer)
{
    boost::crc_32_type result;
    result.process_bytes(buffer.data(), buffer.size());
    uint32_t checksum = result.checksum();
    char crc32[16];
    std::sprintf(crc32, "%8x", checksum);
    return crc32;
}
} // namespace jlst
