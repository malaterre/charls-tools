// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "cjpls_options.h"
#include "tuple.h"

#include "version.h"

#include <boost/program_options.hpp>
#include <iostream>

namespace jlst {

static charls::color_transformation string_to_color_transformation(const char* argument)
{
    if (strcmp(argument, "none") == 0)
        return charls::color_transformation::none;

    if (strcmp(argument, "hp1") == 0)
        return charls::color_transformation::hp1;

    if (strcmp(argument, "hp2") == 0)
        return charls::color_transformation::hp2;

    if (strcmp(argument, "hp3") == 0)
        return charls::color_transformation::hp3;

    throw std::runtime_error("Argument color_transformation needs to be: none, hp1, hp2 or hp3\n");
}

static charls::interleave_mode string_to_interleave_mode(const char* argument)
{
    if (strcmp(argument, "none") == 0)
        return charls::interleave_mode::none;

    if (strcmp(argument, "line") == 0)
        return charls::interleave_mode::line;

    if (strcmp(argument, "sample") == 0)
        return charls::interleave_mode::sample;

    throw std::runtime_error("Argument interleave-mode needs to be: none, line or sample\n");
}

static charls::interleave_mode string_to_planar_configuration(const char* argument)
{
    if (strcmp(argument, "contig") == 0)
        return charls::interleave_mode::sample;

    if (strcmp(argument, "separate") == 0)
        return charls::interleave_mode::none;

    throw std::runtime_error("Argument planar_configuration needs to be: contig or separate\n");
}


bool cjpls_options::process(int argc, char* argv[])
{
    typedef tuple<int, 5> pcp_type;  // preset_coding_parameters
    typedef tuple<int, 2> size_type; // frame_info: width + height
    namespace po = boost::program_options;
    std::string interleave_mode_str;
    std::string color_transformation_str;
    std::string planar_configuration_str;
    pcp_type pcp{};
    size_type size{};
    {
        po::options_description desc("Allowed options");
        desc.add_options()("help,h", "print usage message")                            // help
            ("version", "print version")                                               // version
            ("input,i", po::value(&input)->required(), "Input filename. Required.")    // input
            ("output,o", po::value(&output)->required(), "Output filename. Required ") // output
            ("interleave_mode,m", po::value(&interleave_mode_str),
             "Output interleave mode: `none|line|samples`. "
             "Default to input format if not specified.")                         // interleave mode
            ("near_lossless,n", po::value(&near_lossless), "near lossless value") // near lossless
            ("preset_coding_parameters,k", po::value(&pcp),
             "preset coding parameters: "
             "maximum_sample_value, threshold1, threshold2, threshold3, reset_value") // preset coding parameters
            ("standard_spiff_header", po::value(&standard_spiff_header),
             "Write a standard spiff header: 'yes'/'no'.") // spiff header
            ("color_transformation,t", po::value(&color_transformation_str),
             "Color transformation: `none|hp1|hp2|hp3` (HP extension).") // color transformation
            ("size,s", po::value(&size),
             "Size of image (width, height), unless specified in the format header.") // size width + height
            ("bits_per_sample,b", po::value(&frame_info.bits_per_sample),
             "Bits per sample value, unless specified in the format header.") // bits per sample
            ("component_count,c", po::value(&frame_info.component_count),
             "Component count, unless specified in the format header.") // component count
            ("planar_configuration,p", po::value(&planar_configuration_str),
             "Planar configuration ('contig' or 'separate', unless specified in the format header.") // planar configuration
            ;

        po::positional_options_description p;
        p.add("input", 1);
        p.add("output", 2);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

        if (vm.count("help"))
        {
            std::cout << "usage: cjpls input.ext output.jls\n";
            std::cout << desc << std::endl;
            return false;
        }

        if (vm.count("version"))
        {
            std::cout << "cjpls version: " << JLST_VERSION << "\n";
            std::cout << "charls version: " << charls_get_version_string() << std::endl;
            return false;
        }

        // https://stackoverflow.com/questions/5395503/required-and-optional-arguments-using-boost-library-program-options
        try
        {
            po::notify(vm);
        }
        catch (std::exception&)
        {
            // default value missing. Let's print usage before re-throw:
            std::cout << "usage: cjpls input.ext output.jls\n";
            std::cout << desc << std::endl;
            throw;
        }
        interleave_mode = charls::interleave_mode::none;
        color_transformation = charls::color_transformation::none;
        planar_configuration = charls::interleave_mode::sample;

        if (vm.count("color_transformation"))
        {
            color_transformation = string_to_color_transformation(color_transformation_str.c_str());
        }
        if (vm.count("interleave_mode"))
        {
            interleave_mode = string_to_interleave_mode(interleave_mode_str.c_str());
        }
        if (vm.count("preset_coding_parameters"))
        {
            const int* val = pcp.values;
            preset_coding_parameters.maximum_sample_value = val[0];
            preset_coding_parameters.threshold1 = val[1];
            preset_coding_parameters.threshold2 = val[2];
            preset_coding_parameters.threshold3 = val[3];
            preset_coding_parameters.reset_value = val[4];
        }
        if (vm.count("size"))
        {
            const int* val = size.values;
            frame_info.width = val[0];
            frame_info.height = val[1];
        }
        if (vm.count("planar_configuration"))
        {
            planar_configuration = string_to_planar_configuration(planar_configuration_str.c_str());
        }
    }
    return true;
}
} // namespace jlst
