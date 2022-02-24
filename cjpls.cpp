// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "cjpls_options.h"       // for cjpls_options
#include "factory.h"             // for factory
#include "format.h"              // for format
#include "image.h"               // for image, image_info
#include <charls/public_types.h> // for frame_info
#include <cstdlib>               // for EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>              // for operator<<, endl, basic_ostream, cerr
#include <memory>                // for unique_ptr
#include <stdexcept>             // for invalid_argument
#include <vector>                // for vector
namespace jlst {
class source;
}

static std::unique_ptr<jlst::format> get_format(const jlst::cjpls_options& options, jlst::source& source)
{
    jlst::format* ptr = jlst::factory::instance().get_format_from_type(options.get_type());
    if (!ptr)
        ptr = jlst::factory::instance().detect_format(source);
    if (ptr)
        return std::unique_ptr<jlst::format>(ptr);
    throw std::invalid_argument("no format");
}

static jlst::image combine_images(std::vector<jlst::image> const& images)
{
    if (images.size() == 1)
        return images[0];
    else if (images.size() == 3)
    {
        jlst::image ret;
        for (auto& image : images)
        {
            ret.append(image);
        }
        ret.get_image_info().frame_info().component_count = 3;
        return ret;
    }

    throw std::invalid_argument("combine_images");
}

static void encode(jlst::cjpls_options& options)
{
    auto& sources = options.get_sources();
    std::vector<jlst::image> images;
    for (auto& source : sources)
    {
        auto format = get_format(options, source);
        auto input_image{format->load(source, options.get_image_info())};
        images.push_back(input_image);
    }

    auto image{combine_images(images)};

    std::unique_ptr<jlst::format> jls_format(jlst::factory::instance().get_format_from_type("jls"));
    jls_format->save(options.get_dest(0), image, options.get_jls_options());
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
