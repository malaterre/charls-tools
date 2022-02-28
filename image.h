// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <charls/public_types.h> // for frame_info, interleave_mode, charls...
#include <cstddef>               // for size_t
#include <cstdint>               // for uint8_t
#include <string>                // for string, basic_string
#include <vector>

namespace jlst {
class image_info
{
    charls::frame_info frame_info_{};
    charls::interleave_mode interleave_mode_{};
    std::string comment_{};

public:
    bool invalid();
    bool operator==(const image_info& other) const;

    charls::frame_info& frame_info()
    {
        return frame_info_;
    }
    const charls::frame_info& frame_info() const
    {
        return frame_info_;
    }
    charls::interleave_mode& interleave_mode()
    {
        return interleave_mode_;
    }
    const charls::interleave_mode& interleave_mode() const
    {
        return interleave_mode_;
    }
    std::string& comment()
    {
        return comment_;
    }
    const std::string& comment() const
    {
        return comment_;
    }
};

class image_data
{
    std::size_t stride_{};
    std::vector<uint8_t> pixel_data_{};

public:
    void append(image_data const& id);
    std::size_t& stride()
    {
        return stride_;
    }
    const std::size_t& stride() const
    {
        return stride_;
    }

    std::vector<uint8_t>& pixel_data()
    {
        return pixel_data_;
    }
    const std::vector<uint8_t>& pixel_data() const
    {
        return pixel_data_;
    }
};

class image
{
    image_info image_info_;
    image_data image_data_;

public:
    image_info& get_image_info()
    {
        return image_info_;
    }

    const image_info& get_image_info() const
    {
        return image_info_;
    }

    image_data& get_image_data()
    {
        return image_data_;
    }

    const image_data& get_image_data() const
    {
        return image_data_;
    }

    void append(image const& other);

    std::vector<uint8_t> transform(charls::interleave_mode const& interleave_mode) const;

    std::vector<uint8_t> crop(uint32_t X, uint32_t Y, uint32_t width, uint32_t height);
    std::vector<uint8_t> flip(bool vertical); // horizontal when false
    std::vector<uint8_t> rotate(int degree);
    std::vector<uint8_t> transpose();
    std::vector<uint8_t> transverse();
    std::vector<uint8_t> wipe(uint32_t X, uint32_t Y, uint32_t width, uint32_t height);
};

} // namespace jlst
