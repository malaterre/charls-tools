// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "djpls_options.h"
#include "pnm.h"
#include "raw.h"

#include <fstream>
#include <iostream>
#include <vector>

static jlst::format& get_format(const jlst::djpls_options& options)
{
    using refformat = std::reference_wrapper<jlst::format>;
    static const refformat formats[] = {jlst::pnm::get(), jlst::raw::get()};

    for (jlst::format& format : formats)
    {
        if (format.detect2(options))
        {
            return format;
        }
    }
    throw std::invalid_argument("no format");
}

static std::vector<uint8_t> read_file(std::string const& filename)
{
    std::ifstream input;
    input.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);
    input.open(filename.c_str(), std::ios::in | std::ios::binary);

    input.seekg(0, std::ios::end);
    const auto byte_count_file = static_cast<size_t>(input.tellg());
    input.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(byte_count_file);
    input.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

    return buffer;
}

static void decode(const jlst::djpls_options& options)
{
    jlst::format& format = get_format(options);

    const std::vector<uint8_t> encoded_source = read_file(options.input.c_str());
    charls::jpegls_decoder decoder;
    decoder.source(encoded_source);
    decoder.read_header();

    const charls::frame_info& fi = decoder.frame_info();
    std::vector<uint8_t> decoded_buffer(decoder.destination_size());
    decoder.decode(decoded_buffer);

#if 1
    format.open(options.output.c_str(), true);
    format.get_info() = decoder.frame_info();
    format.get_mode() = decoder.interleave_mode();
    format.get_stride() = 0; // FIXME ?
    format.write_info();
    format.write_data(decoded_buffer.data(), decoded_buffer.size());
#else
    std::ofstream output(options.output.c_str(), std::ios::binary);
    output.exceptions(std::ios::failbit | std::ios::badbit);

    if (fi.component_count == 1)
        output << "P5\n";
    else
        output << "P6\n";
    output << fi.width << " " << fi.height << '\n';
    output << ((1 << fi.bits_per_sample) - 1) << '\n';
    output.write(reinterpret_cast<char*>(decoded_buffer.data()), decoded_buffer.size());
#endif

    format.close();
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
