// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <charls/charls.h>
#include <vector>

namespace jlst {
class format;
class source;
class image_info
{
    charls::frame_info frame_info_{};
    charls::interleave_mode interleave_mode_{};
    std::string comment_{};

public:
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

    void load(const jlst::format& format, jlst::source& source);

    std::vector<uint8_t> transform(charls::interleave_mode const& interleave_mode) const;
};

} // namespace jlst
