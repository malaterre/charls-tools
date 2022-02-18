// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "jls.h"
#include "image.h"
#include "utils.h"

#include <fstream>
#include <limits>
#include <sstream>
#include <vector>

namespace jlst {
bool jls::detect(source& s, image_info const& ii) const
{
    return false;
}
bool jls::detect2(const djpls_options&) const
{
    return true;
}

void jls::read_info(source& fs, image& i) const
{
}

void jls::read_data(source& fs, image& i) const
{
}

namespace {
static std::vector<uint8_t> compress(jlst::image const& image, const jlst::jls_options& options)
{
    auto const& frame_info = image.get_image_info().frame_info();
    // what if user requested 'line' or 'sample' for single component ? Let's
    // handle it here (not sure why charls does not handle it internally).
    //    auto& options = opt.get_jls_options();
    if (frame_info.component_count == 1)
    {
        if (options.interleave_mode != charls::interleave_mode::none)
            throw std::invalid_argument("Invalid interleave_mode for single component. Use 'none'");
    }

    charls::jpegls_encoder encoder;

    // setup encoder using compressor's options:
    encoder
        .interleave_mode(options.interleave_mode)                   // interleave_mode
        .near_lossless(options.near_lossless)                       // near_lossless
        .preset_coding_parameters(options.preset_coding_parameters) // preset_coding_parameters
        .color_transformation(options.color_transformation);        // color_transformation

    // setup encoder using input image:
    encoder.frame_info(frame_info); // frame_info

    // allocate output:
    std::vector<uint8_t> buffer(encoder.estimated_destination_size());
    encoder.destination(buffer);
    // now that destination buffer is set, write SPIFF header:
    if (options.standard_spiff_header)
    {
        const charls::spiff_color_space spiff_color_space =
            frame_info.component_count == 1 ? charls::spiff_color_space::grayscale : charls::spiff_color_space::rgb;
        encoder.write_standard_spiff_header(spiff_color_space);
    }

#if CHARLS_VERSION_MAJOR > 2 || (CHARLS_VERSION_MAJOR == 2 && CHARLS_VERSION_MINOR > 2)
    {
        // the following writes an extra \0
        // encoder.write_comment(image.get_image_info().comment().c_str());
        auto& comment = image.get_image_info().comment();
        encoder.write_comment(comment.c_str(), comment.size());
    }
#endif

    const auto transform_pixel_data{image.transform(options.interleave_mode)};
    size_t encoded_size;
    if (options.interleave_mode == charls::interleave_mode::none)
    {
        encoded_size = encoder.encode(transform_pixel_data);
    }
    else
    {
        encoded_size = encoder.encode(transform_pixel_data, image.get_image_data().stride());
    }
    buffer.resize(encoded_size);

    return buffer;
}
} // end namespace

void jls::write_info(dest& d, const image& i, const jls_options& jo) const
{
}

void jls::write_data(dest& fs, const image& i, const jls_options& jo) const
{
    auto encoded_buffer{compress(i, jo)};
    fs.write(encoded_buffer.data(), encoded_buffer.size());
}

const format& jls::get()
{
    static const jls jls_;
    return jls_;
}
} // namespace jlst
