// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "image.h"
#include "format.h"
#include "utils.h"

#include <cassert>

namespace jlst {

void image::load(const jlst::format& format, jlst::source& source)
{
    auto& image_info = this->get_image_info();
    auto& image_data = this->get_image_data();
    format.read_info(source, *this);
    auto const& info = image_info.frame_info();
    auto& pixel_data = image_data.pixel_data();
    auto const bytes_per_sample{(info.bits_per_sample + 7) / 8};
    pixel_data.resize(info.width * info.height * bytes_per_sample * info.component_count);
    format.read_data(source, *this);
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
