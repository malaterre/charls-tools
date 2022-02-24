// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "format.h"
namespace jlst {
image format::load(jlst::source& source, image_info const& ii) const
{
    image ret;

    auto& image_info = ret.get_image_info();
    auto& image_data = ret.get_image_data();

    // raw codec is a bit special, since header info is specifed by the
    // user on the command line. We must initialize the default image info
    // here, all other codec, will simply override user defaults
    ret.get_image_info() = ii;

    this->read_info(source, ret);
    auto const& info = image_info.frame_info();
    auto& pixel_data = image_data.pixel_data();
    auto const bytes_per_sample{(info.bits_per_sample + 7) / 8};
    pixel_data.resize(info.width * info.height * bytes_per_sample * info.component_count);
    this->read_data(source, ret);

    return ret;
}
void format::save(jlst::dest& dest, image const& i, jls_options const& options) const
{
    this->write_info(dest, i, options);
    this->write_data(dest, i, options);
}
} // end namespace jlst
