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

jlst::image load_image(jlst::format& format, const jlst::cjpls_options& options)
{
    jlst::image input_image{};
    format.open(options.input.c_str(), false);
    format.read_info();
    input_image.frame_info() = format.get_info();
    input_image.interleave_mode() = format.get_mode();
    input_image.stride() = format.get_stride();
    auto const& info = input_image.frame_info();
    auto& pixel_data = input_image.pixel_data();
    pixel_data.resize(info.width * info.height * (info.bits_per_sample / 8) * info.component_count);
    format.read_data(pixel_data.data(), pixel_data.size());

    return input_image;
}

std::vector<uint8_t> compress(jlst::image const& image, const jlst::cjpls_options& options)
{
    auto const& frame_info = image.frame_info();
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
        encoded_size = encoder.encode(transform_pixel_data, image.stride());
    }
    buffer.resize(encoded_size);

    return buffer;
}

static jlst::format& get_format(const jlst::cjpls_options& options)
{
    using refformat = std::reference_wrapper<jlst::format>;
    static const refformat formats[] = {jlst::pnm::get(), jlst::raw::get()};

    for (jlst::format& format : formats)
    {
        if (format.detect(options))
        {
            return format;
        }
    }
    throw std::invalid_argument("no format");
}

static void encode(const jlst::cjpls_options& options)
{
    jlst::format& format = get_format(options);

    auto image{load_image(format, options)};
    auto encoded_buffer{compress(image, options)};
    jlst::utils::save_buffer_to_file(encoded_buffer.data(), encoded_buffer.size(), options.output.c_str());
    format.close();
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
