// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "image.h"
#include "jls.h"
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

static const jlst::format& get_format(const jlst::cjpls_options& options, jlst::source& source)
{
    using refformat = std::reference_wrapper<const jlst::format>;
    static refformat formats[] = {jlst::pnm::get(), jlst::raw::get()};

    for (const jlst::format& format : formats)
    {
        if (format.detect(source, options.get_image_info()))
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
        const jlst::format& format = get_format(options, source);
        auto input_image{format.load(source, options.get_image_info())};
        images.push_back(input_image);
    }

    auto image{combine_images(images)};

    jlst::jls::get().save(options.get_dest(0), image, options.get_jls_options());
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
