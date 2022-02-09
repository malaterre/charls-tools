// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "djpls_options.h"
#include "tuple.h"

#include "version.h"

#include <boost/program_options.hpp>
#include <iostream>

namespace jlst {

static charls::interleave_mode string_to_planar_configuration(const char* argument)
{
    if (strcmp(argument, "contig") == 0)
        return charls::interleave_mode::sample;

    if (strcmp(argument, "separate") == 0)
        return charls::interleave_mode::none;

    throw std::runtime_error("Argument planar_configuration needs to be: contig or separate\n");
}

bool djpls_options::process(int argc, char* argv[])
{
    std::vector<std::string> inputs{};
    std::vector<std::string> outputs{};
    namespace po = boost::program_options;
    std::string planar_configuration_str;
    {
        po::options_description desc("Allowed options");
        desc.add_options()("help,h", "print usage message")                                  // help
            ("version", "print version")                                                     // version
            ("input,i", po::value(&inputs) /*->required()*/, "Input filename. Required.")    // input
            ("output,o", po::value(&outputs) /*->required()*/, "Output filename. Required ") // output
            ("planar_configuration,p", po::value(&planar_configuration_str),
             "Planar configuration ('contig' or 'separate', unless specified in the format header.") // planar configuration
            ;

        po::positional_options_description p;
        // do not allow more than one positional arg for now:
        p.add("input", 1);
        p.add("output", 1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

        if (vm.count("help"))
        {
            std::cout << "usage: djpls input.jls output.ext\n";
            std::cout << desc << std::endl;
            return false;
        }

        if (vm.count("version"))
        {
            std::cout << "djpls version: " << JLST_VERSION << "\n";
            std::cout << "charls version: " << charls_get_version_string() << std::endl;
            return false;
        }

        // https://stackoverflow.com/questions/5395503/required-and-optional-arguments-using-boost-library-program-options
        try
        {
            po::notify(vm);
            if (vm.count("input"))
            {
                add_inputs(inputs);
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
            std::cout << "usage: djpls input.jls output.ext\n";
            std::cout << desc << std::endl;
            throw;
        }

        planar_configuration = charls::interleave_mode::sample;

        if (vm.count("planar_configuration"))
        {
            planar_configuration = string_to_planar_configuration(planar_configuration_str.c_str());
        }
    }
    return true;
}
} // namespace jlst
