// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "cjpls_options.h"
#include "tuple.h"

//#include "source.h"
#include "version.h"

#include <boost/program_options.hpp>
#include <fstream>

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
    std::vector<std::string> inputs{};
    std::vector<std::string> outputs{};
    typedef tuple<int, 5> pcp_type;  // preset_coding_parameters
    typedef tuple<int, 2> size_type; // frame_info: width + height
    std::string interleave_mode_str{};
    std::string color_transformation_str{};
    std::string planar_configuration_str{};
    pcp_type pcp{};
    size_type size{};
    auto& frame_info = image_info_.frame_info();
    auto& planar_configuration = image_info_.interleave_mode();
    {
        namespace po = boost::program_options;
        po::options_description generic("Generic options (required when no redirects)");
        generic.add_options()                                                      //
            ("input,i", po::value(&inputs) /*->required()*/, "Input filename.")    // input
            ("output,o", po::value(&outputs) /*->required()*/, "Output filename.") // output
            ("type", po::value(&type_), "Input type (pgm, raw...).")               // input type
            ;

        po::options_description jpegls("JPEG-LS output options");
        jpegls.add_options() //
            ("interleave_mode,m", po::value(&interleave_mode_str),
             "Output interleave mode: `none|line|sample`. "
             "Default to input format if not specified.")                                      // interleave mode
            ("near_lossless,n", po::value(&jls_options_.near_lossless), "near lossless value") // near lossless
            ("preset_coding_parameters,k", po::value(&pcp),
             "preset coding parameters: "
             "maximum_sample_value, threshold1, threshold2, threshold3, reset_value") // preset coding parameters
            ("standard_spiff_header", po::value(&jls_options_.standard_spiff_header),
             "Write a standard spiff header: 'yes'/'no'.") // spiff header
            ("color_transformation,t", po::value(&color_transformation_str),
             "Color transformation: `none|hp1|hp2|hp3` (HP extension).") // color transformation
            ;

        po::options_description image("Image input options");
        image.add_options() //
            ("size,s", po::value(&size),
             "Size of image (width, height), unless specified in the format header.") // size width + height
            ("bits_per_sample,b", po::value(&frame_info.bits_per_sample),
             "Bits per sample value, unless specified in the format header.") // bits per sample
            ("component_count,c", po::value(&frame_info.component_count),
             "Component count, unless specified in the format header.") // component count
            ("planar_configuration,p", po::value(&planar_configuration_str),
             "Planar configuration ('contig' or 'separate'), unless specified in the format header.") // planar configuration
            ;

        po::options_description encoding("Encoding options");
        encoding.add_options()                                         //
            ("even_destination_size", "even destination size")         //
            ("include_version_number", "include version number")       //
            ("include_pc_parameters_jai", "include pc parameters jai") //
            ;

        po::options_description desc("Allowed options");
        desc.add_options()("help,h", "print usage message") // help
            ("version", "print version")                    // version
            ;

        desc.add(generic);
        desc.add(jpegls);
        desc.add(encoding);
        desc.add(image);

        po::positional_options_description p;
        // do not allow more than one positional arg for now:
        p.add("input", 1);
        p.add("output", 1);
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

            // let's pretend that input/output are actually required:
            if (vm.count("input"))
            {
                add_inputs(inputs);
                if (!vm.count("type"))
                {
                    type_ = compute_type_from_filenames(inputs);
                }
            }
            else
            {
                add_stdin_input();
            }

            if (vm.count("output"))
            {
                add_outputs(outputs);
            }
            else
            {
                add_stdout_output();
            }
        }
        catch (std::exception&)
        {
            // default value missing. Let's print usage before re-throw:
            std::cout << "usage: cjpls input.ext output.jls\n";
            std::cout << desc << std::endl;
            throw;
        }

        jls_options_.interleave_mode = charls::interleave_mode::none;
        jls_options_.color_transformation = charls::color_transformation::none;
        planar_configuration = charls::interleave_mode::sample;

        if (vm.count("color_transformation"))
        {
            jls_options_.color_transformation = string_to_color_transformation(color_transformation_str.c_str());
        }
        if (vm.count("interleave_mode"))
        {
            jls_options_.interleave_mode = string_to_interleave_mode(interleave_mode_str.c_str());
        }
        if (vm.count("preset_coding_parameters"))
        {
            const int* val = pcp.values;
            jls_options_.preset_coding_parameters.maximum_sample_value = val[0];
            jls_options_.preset_coding_parameters.threshold1 = val[1];
            jls_options_.preset_coding_parameters.threshold2 = val[2];
            jls_options_.preset_coding_parameters.threshold3 = val[3];
            jls_options_.preset_coding_parameters.reset_value = val[4];
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
        if (vm.count("even_destination_size"))
        {
#if CHARLS_VERSION_MAJOR > 2 || (CHARLS_VERSION_MAJOR == 2 && CHARLS_VERSION_MINOR > 2)
            jls_options_.encoding_options = jls_options_.encoding_options | charls::encoding_options::even_destination_size;
#else
            throw std::runtime_error("Argument even_destination_size is unhandled.\n");
#endif
        }
        if (vm.count("include_version_number"))
        {
#if CHARLS_VERSION_MAJOR > 2 || (CHARLS_VERSION_MAJOR == 2 && CHARLS_VERSION_MINOR > 2)
            jls_options_.encoding_options = jls_options_.encoding_options | charls::encoding_options::include_version_number;
#else
            throw std::runtime_error("Argument include_version_number is unhandled.\n");
#endif
        }
        if (vm.count("include_pc_parameters_jai"))
        {
#if CHARLS_VERSION_MAJOR > 2 || (CHARLS_VERSION_MAJOR == 2 && CHARLS_VERSION_MINOR > 2)
            jls_options_.encoding_options =
                jls_options_.encoding_options | charls::encoding_options::include_pc_parameters_jai;
#else
            throw std::runtime_error("Argument include_pc_parameters_jai is unhandled.\n");
#endif
        }
    }
    return true;
}


} // namespace jlst
