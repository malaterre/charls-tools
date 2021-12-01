// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <charls/charls.h>
#include <vector>

namespace jlst {
class image
{
    charls::frame_info frame_info_{};
    charls::interleave_mode interleave_mode_{};
    std::size_t stride_{};
    std::vector<uint8_t> pixel_data_{};

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
    std::vector<uint8_t> transform(charls::interleave_mode const& interleave_mode) const;
};

} // namespace jlst
