// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "image.h"
#include "format.h"
#include "utils.h"

#include <cassert>
#include <cstring>

namespace jlst {

bool image_info::invalid()
{
    return frame_info_.width == 0;
}
inline bool operator==(const charls::frame_info& lhs, const charls::frame_info& rhs)
{
    return lhs.width == rhs.width && lhs.height == rhs.height && lhs.bits_per_sample == rhs.bits_per_sample &&
           lhs.component_count == rhs.component_count;
}
bool image_info::operator==(const image_info& other) const
{
    const bool b1 = frame_info_ == other.frame_info_;
    const bool b2 = interleave_mode_ == other.interleave_mode_;
    return b1 && b2;
}
void image_data::append(image_data const& id)
{
    pixel_data_.insert(pixel_data_.end(), id.pixel_data_.begin(), id.pixel_data_.end());
}

void image::append(image const& other)
{
    if (image_info_.invalid())
    {
        image_info_ = other.image_info_;
        image_data_ = other.image_data_;
    }
    else
    {
        if (image_info_ == other.image_info_)
        {
            image_data_.append(other.image_data_);
        }
    }
}

std::vector<uint8_t> image::transform(charls::interleave_mode const& interleave_mode) const
{
    auto& frame_info = get_image_info().frame_info();
    if (frame_info.component_count == 1 || get_image_info().interleave_mode() == interleave_mode)
        return get_image_data().pixel_data();
    if (frame_info.component_count == 3)
    {
        if (interleave_mode == charls::interleave_mode::none)
        {
            assert(get_image_info().interleave_mode() == charls::interleave_mode::sample);
            return utils::triplet_to_planar(get_image_data().pixel_data(), frame_info.width, frame_info.height,
                                            frame_info.bits_per_sample, get_image_data().stride());
        }
        else if (interleave_mode == charls::interleave_mode::line)
        {
            if (get_image_info().interleave_mode() == charls::interleave_mode::sample)
                return get_image_data().pixel_data();
            assert(get_image_info().interleave_mode() == charls::interleave_mode::none);
            return utils::planar_to_triplet(get_image_data().pixel_data(), frame_info.width, frame_info.height,
                                            frame_info.bits_per_sample, get_image_data().stride());
        }
        else if (interleave_mode == charls::interleave_mode::sample)
        {
            assert(get_image_info().interleave_mode() == charls::interleave_mode::none);
            return utils::planar_to_triplet(get_image_data().pixel_data(), frame_info.width, frame_info.height,
                                            frame_info.bits_per_sample, get_image_data().stride());
        }
    }
    throw std::invalid_argument("invalid transform request");
}

} // namespace jlst
