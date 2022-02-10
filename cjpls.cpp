// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "image.h"
#include "pnm.h"
#include "raw.h"
#include "utils.h"
#include "version.h"

#include <cassert>
#include <cstdlib> // EXIT_SUCCESS
#include <fstream>
#include <iostream>
#include <vector>

#include <charls/charls.h>

#include "cjpls_options.h"

static jlst::image load_image(const jlst::format& format, jlst::source& source)
{
    jlst::image input_image{};
    input_image.load(format, source);
    return input_image;
}

static std::vector<uint8_t> compress(jlst::image const& image, const jlst::cjpls_options& options)
{
    auto const& frame_info = image.get_image_info().frame_info();
    // what if user requested 'line' or 'sample' for single component ? Let's
    // handle it here (not sure why charls does not handle it internally).
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

static const jlst::format& get_format(const jlst::cjpls_options& options, jlst::source& source)
{
    using refformat = std::reference_wrapper<const jlst::format>;
    static refformat formats[] = {jlst::pnm::get(), jlst::raw::get()};

    for (const jlst::format& format : formats)
    {
        if (format.detect(source))
        {
            return format;
        }
    }
    throw std::invalid_argument("no format");
}

static jlst::image combine_images(std::vector<jlst::image> const& images)
{
    if (images.size() == 1)
        return images[0];
    else if (images.size() == 3)
    {
        throw std::invalid_argument("todo");
    }

    throw std::invalid_argument("combine_images");
}

static void encode(jlst::cjpls_options& options)
{
    auto& sources = options.get_sources();
    std::vector<jlst::image> images;
    for (auto& source : sources)
    {
        const jlst::format& format = get_format(options, source);
        auto image{load_image(format, source)};
        images.push_back(image);
    }

    auto image{combine_images(images)};

    auto encoded_buffer{compress(image, options)};
    options.get_dest(0).write(encoded_buffer.data(), encoded_buffer.size());
}

int main(int argc, char* argv[])
{
    jlst::cjpls_options options{};
    try
    {
        if (!options.process(argc, argv))
        {
            // help, or version requested. Return without error
            return EXIT_SUCCESS;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Invalid options: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "unknown exception during options parsing" << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        encode(options);
    }
    catch (std::exception& e)
    {
        std::cerr << "Error during encoding: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "unknown exception during encoding" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
