// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "jplsinfo_options.h"
#include "version.h"
#include <charls/charls.h>

#include <boost/program_options.hpp>
#include <iostream>

namespace jlst {

bool info_options::process(int argc, char* argv[])
{
    namespace po = boost::program_options;
    {
        po::options_description desc("Allowed options");
        std::string hash_name;
        std::vector<std::string> inputs{};
        std::vector<std::string> outputs{};
        // by default unix_style includes `allow_guessing`, so that user can use abbreviation:
        desc.add_options()("help,h", "print usage message")                       // help
            ("version", "print version")                                          // version
            ("input,i", po::value(&inputs) /*->required()*/, "inputs. Required.") // input
            ("output,o", po::value(&outputs) /*->required()*/, "outputs.")        // output
            ("format,f", po::value(&format), "format")                            // json/xml/yaml
            ("pretty", "prettify output")                                         // pretty
            ("hash", po::value(&hash_name), "use hash (eg. 'crc32')")             // compute hash of decoded buffer
            ;

        po::positional_options_description p;
        p.add("input", -1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

        if (vm.count("help"))
        {
            std::cout << "usage: jplsinfo [options] input\n";
            std::cout << desc << "\n";
            return false;
        }

        if (vm.count("version"))
        {
            std::cout << "jplsinfo version: " << JLST_VERSION << "\n";
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
            std::cout << "usage: jplsinfo [options] input\n";
            std::cout << desc << std::endl;
            throw;
        }

        if (vm.count("pretty"))
        {
            pretty = true;
        }
        if (vm.count("hash"))
        {
            if (hash_name == "crc32")
            {
                with_hash = true;
            }
            else
            {
                throw std::invalid_argument("hash: " + hash_name);
            }
        }
    } // namespace boost::program_options;
    return true;
}
} // namespace jlst
