// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "cjpls_options.h"
#include "factory.h" // for factory
#include "format.h"  // for format
#include "image.h"   // for image, image_info
#include "jls.h"     // for format
#include "jplstran_options.h"

#include <iostream> // for operator<<, endl, basic_ostream, cerr
#include <memory>   // for unique_ptr

static void transform(jlst::tran_options& options)
{
    jlst::image input_image;
#if 0
    std::unique_ptr<jlst::format> jls_format(jlst::factory::instance().get_format_from_type("jls"));
    input_image = jls_format->load(options.get_source(0), input_image.get_image_info());

    jlst::jls_options jo{};
    jls_format->save(options.get_dest(0), input_image, jo);
#else
    jlst::jls* ptr = new jlst::jls;
    std::unique_ptr<jlst::jls> jls_format(ptr);
    jls_format->transform(options.get_dest(0), options.get_source(0), options);
#endif
}

int main(int argc, char* argv[])
{
    jlst::tran_options options{};
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
        transform(options);
    }
    catch (std::exception& e)
    {
        std::cerr << "Invalid options: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "unknown exception during transform" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
