// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "djpls_options.h"
#include "factory.h"
#include "image.h"
#include "jls.h"
#include "pnm.h"
#include "raw.h"

#include <fstream>
#include <iostream>
#include <vector>

// compute output format (do not inspect source)
static std::unique_ptr<jlst::format> get_format(jlst::djpls_options& options)
{
    jlst::format* ptr = jlst::factory::instance().get_format_from_type(options.get_type());
    if (ptr)
        return std::unique_ptr<jlst::format>(ptr);

    throw std::invalid_argument("no format");
}

static void decode(jlst::djpls_options& options)
{
#if 0
    const std::vector<uint8_t> encoded_source = options.get_source(0).read_bytes();
    charls::jpegls_decoder decoder;
    decoder.source(encoded_source);
    // comment handling, must be setup before any read_* function
    std::string comment;
#if CHARLS_VERSION_MAJOR > 2 || (CHARLS_VERSION_MAJOR == 2 && CHARLS_VERSION_MINOR > 2)
    {
        decoder.at_comment([&comment](const void* data, const size_t size) noexcept {
            comment = std::string(static_cast<const char*>(data), size);
        });
    }
#endif
    decoder.read_header();

    const charls::frame_info& fi = decoder.frame_info();
    std::vector<uint8_t> decoded_buffer(decoder.destination_size());
    decoder.decode(decoded_buffer);

    jlst::image input_image;
    input_image.get_image_info().frame_info() = decoder.frame_info();
    input_image.get_image_info().interleave_mode() = decoder.interleave_mode();
    input_image.get_image_info().comment() = comment;
    input_image.get_image_data().pixel_data() = decoded_buffer;
#else
    jlst::image input_image;
    std::unique_ptr<jlst::format> jls_format(jlst::factory::instance().get_format_from_type("jls"));
    input_image = jls_format->load(options.get_source(0), input_image.get_image_info());
#endif

    auto format = get_format(options);
    jlst::jls_options jo;
    format->write_info(options.get_dest(0), input_image, jo);
    format->write_data(options.get_dest(0), input_image, jo);
}

int main(int argc, char* argv[])
{
    jlst::djpls_options options{};
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
        decode(options);
    }
    catch (std::exception& e)
    {
        std::cerr << "Error during decoding: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "unknown exception during decoding" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
