// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "jplstran_options.h"
#include "tuple.h"
#include "version.h"
#include <charls/charls.h>

#include <boost/program_options.hpp>
#include <iostream>

namespace jlst {

bool tran_options::process(int argc, char* argv[])
{
    typedef tuple<int, 4> region_type; // region type
    namespace po = boost::program_options;
    {
        po::options_description desc("Allowed options");
        std::vector<std::string> inputs{};
        std::vector<std::string> outputs{};
        region_type region_tuple;
        std::string flip;
        // by default unix_style includes `allow_guessing`, so that user can use abbreviation:
        desc.add_options()("help,h", "print usage message")                       // help
            ("version", "print version")                                          // version
            ("input,i", po::value(&inputs) /*->required()*/, "inputs. Required.") // input
            ("output,o", po::value(&outputs) /*->required()*/, "outputs.")        // output
            ("crop", po::value(&region_tuple), "crop WxH+X+Y")                    // crop
            ("flip", po::value(&flip), "flip horizontal|vertical")                // flip
            ("rotate", po::value(&degree), "rotate 90|180|270")                   // rotate
            ("transpose", "transpose")                                            // transpose
            ("transverse", "transverse")                                          // transverse
            ("wipe", po::value(&region_tuple), "wipe WxH+X+Y")                    // crop
            ;

        po::positional_options_description p;
        // do not allow more than one positional arg for now:
        p.add("input", 1);
        p.add("output", 1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

        if (vm.count("help"))
        {
            std::cout << "usage: jplstran [options] input.jls output.jls\n";
            std::cout << desc << std::endl;
            return false;
        }

        if (vm.count("version"))
        {
            std::cout << "jplstran version: " << JLST_VERSION << "\n";
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
                add_stdout_output(false);
            }
        }
        catch (std::exception&)
        {
            // default value missing. Let's print usage before re-throw:
            std::cout << "usage: jplstran [options] input.jls output.jls\n";
            std::cout << desc << std::endl;
            throw;
        }

        if (vm.count("crop"))
        {
            type = transform_type::crop;
            region.Width = region_tuple.values[0];
            region.Height = region_tuple.values[1];
            region.X = region_tuple.values[2];
            region.Y = region_tuple.values[3];
        }
        else if (vm.count("flip"))
        {
            type = transform_type::flip;
            if (flip == "vertical")
            {
                vertical = true;
            }
            else if (flip == "horizontal")
            {
                vertical = false;
            }
            else
            {
                throw std::runtime_error("flip:" + flip);
            }
        }
        else if (vm.count("rotate"))
        {
            type = transform_type::rotate;
            if (degree != 90 && degree != 180 && degree != 270)
                throw std::runtime_error("rotate:" + degree);
        }
        else if (vm.count("transpose"))
            type = transform_type::transpose;
        else if (vm.count("transverse"))
            type = transform_type::transverse;
        else if (vm.count("wipe"))
        {
            type = transform_type::wipe;
            region.Width = region_tuple.values[0];
            region.Height = region_tuple.values[1];
            region.X = region_tuple.values[2];
            region.Y = region_tuple.values[3];
        }
    } // namespace boost::program_options;
    return true;
}
} // namespace jlst
