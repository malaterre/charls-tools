// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "image.h"
#include "utils.h"

#include <cassert>

namespace jlst {

std::vector<uint8_t> image::transform(charls::interleave_mode const& interleave_mode) const
{
    if (frame_info_.component_count == 1 || interleave_mode == interleave_mode_)
        return pixel_data();
    if (frame_info_.component_count == 3)
    {
        if (interleave_mode == charls::interleave_mode::none)
        {
            assert(interleave_mode_ == charls::interleave_mode::sample);
            return utils::triplet_to_planar(pixel_data_, frame_info_.width, frame_info_.height, frame_info_.bits_per_sample,
                                            stride_);
        }
        else if (interleave_mode == charls::interleave_mode::line)
        {
            if (interleave_mode_ == charls::interleave_mode::sample)
                return pixel_data();
            assert(interleave_mode_ == charls::interleave_mode::none);
            return utils::planar_to_triplet(pixel_data_, frame_info_.width, frame_info_.height, frame_info_.bits_per_sample,
                                            stride_);
        }
        else if (interleave_mode == charls::interleave_mode::sample)
        {
            assert(interleave_mode_ == charls::interleave_mode::none);
            return utils::planar_to_triplet(pixel_data_, frame_info_.width, frame_info_.height, frame_info_.bits_per_sample,
                                            stride_);
        }
    }
    throw std::invalid_argument("TODO");
}

} // namespace jlst
