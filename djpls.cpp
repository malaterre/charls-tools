// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "cjpls_options.h"
#include "djpls_options.h"
#include "factory.h"
#include "image.h"
#include "jls.h"
#include "pnm.h"
#include "raw.h"

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
    jlst::image input_image;
    std::unique_ptr<jlst::format> jls_format(jlst::factory::instance().get_format_from_type("jls"));
    input_image = jls_format->load(options.get_source(0), input_image.get_image_info());

    auto format = get_format(options);
    jlst::jls_options jo{};
    format->save(options.get_dest(0), input_image, jo);
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
