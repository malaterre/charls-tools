// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "crc32.h"
#include "jplsinfo_options.h"
#include <charls/charls.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

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

struct writer
{
    writer(bool pretty) : pretty_(pretty)
    {
    }
    virtual ~writer()
    {
    }
    virtual void print_header(std::ostream& os, std::string const& header) = 0;
    virtual void print_footer(std::ostream& os, std::string const& footer) = 0;
    virtual void print_value_separator(std::ostream& os, bool eol) = 0;
    template<typename T>
    void print_value(std::ostream& os, std::string const& key, const T& val)
    {
        std::stringstream ss;
        ss << val;
        const bool is_integral = std::is_integral<T>::value;
        print_tab(os);
        print_string(os, key, ss.str(), is_integral);
    }
    virtual void print_string(std::ostream& os, std::string const& key, std::string const& val, bool is_integral) = 0;
    bool pretty()
    {
        return pretty_;
    }

    int indent_level()
    {
        return indent_level_;
    }
    bool root()
    {
        return indent_level_ == 0;
    }
    void push()
    {
        indent_level_++;
    }
    void pop()
    {
        indent_level_--;
    }
    void print_tab(std::ostream& os)
    {
        if (!pretty())
            return;
        for (int i = 0; i < indent_level_; ++i)
        {
            os << "  ";
        }
    }

private:
    bool pretty_{};
    int indent_level_{};
};

struct yaml_writer : writer
{
    yaml_writer(bool) : writer(true)
    {
        pop();
    }
    ~yaml_writer()
    {
    }
    void print_header(std::ostream& os, std::string const& header)
    {
        if (indent_level() < 0)
        {
            os << "---";
        }
        print_tab(os);
        if (!header.empty())
        {
            os << header;
            os << ':';
        }
        os << '\n';
        push();
    }
    void print_footer(std::ostream& os, std::string const&)
    {
        pop();
        print_tab(os);
    }
    void print_value_separator(std::ostream&, bool)
    {
    }
    void print_string(std::ostream& os, std::string const& key, std::string const& val, bool)
    {
        os << key;
        os << ':';
        os << ' ';
        os << val;
        os << '\n';
    }

private:
};

struct json_writer : writer
{
    json_writer(bool pretty) : writer(pretty)
    {
    }
    ~json_writer()
    {
    }
    void print_header(std::ostream& os, std::string const& header)
    {
        print_tab(os);
        if (!header.empty())
        {
            os << '"' << header << '"';
            if (pretty())
                os << ' ';
            os << ':';
            if (pretty())
                os << ' ';
        }
        os << '{';
        if (pretty())
            os << '\n';
        push();
    }
    void print_footer(std::ostream& os, std::string const&)
    {
        pop();
        print_tab(os);
        os << '}';
    }
    void print_value_separator(std::ostream& os, bool eol)
    {
        if (!eol)
            os << ",";
        if (pretty())
            os << '\n';
    }
    void print_string(std::ostream& os, std::string const& key, std::string const& val, bool is_integral)
    {
        os << '"' << key << '"';
        if (pretty())
            os << ' ';
        os << ':';
        if (pretty())
            os << ' ';
        if (!is_integral)
            os << '"';
        os << val;
        if (!is_integral)
            os << '"';
    }

private:
};

struct xml_writer : writer
{
    xml_writer(bool pretty) : writer(pretty)
    {
    }
    ~xml_writer()
    {
    }
    void print_header(std::ostream& os, std::string const& header)
    {
        if (root())
        {
            os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
            if (pretty())
                os << '\n';
        }
        print_tab(os);
        if (header.empty())
            os << "<charls>";
        else
            os << '<' << header << '>';
        if (pretty())
            os << '\n';
        push();
    }
    void print_footer(std::ostream& os, std::string const& header)
    {
        pop();
        print_tab(os);
        if (header.empty())
            os << "</charls>";
        else
            os << '<' << '/' << header << '>';
    }
    void print_value_separator(std::ostream& os, bool)
    {
        if (pretty())
            os << '\n';
    }
    void print_string(std::ostream& os, std::string const& key, std::string const& val, bool)
    {
        os << '<' << key << '>' << val;
        os << '<' << '/' << key << '>';
    }

private:
};


#define TYPECASE(TYPE, ENUM) \
    case charls::TYPE::ENUM: \
        return #ENUM

#define COLORCASE(X) TYPECASE(spiff_color_space, X)
#define PROFILECASE(X) TYPECASE(spiff_profile_id, X)
#define COMPCASE(X) TYPECASE(spiff_compression_type, X)
#define RESCASE(X) TYPECASE(spiff_resolution_units, X)
#define MODECASE(X) TYPECASE(interleave_mode, X)
#define COLTRANCASE(X) TYPECASE(color_transformation, X)


// TODO:
// https://stackoverflow.com/questions/9183993/msvc-variadic-macro-expansion

static const char* spiff_color_space_to_string(const charls::spiff_color_space& color_space)
{
    switch (color_space)
    {
        COLORCASE(bi_level_black);
        COLORCASE(ycbcr_itu_bt_709_video);
        COLORCASE(none);
        COLORCASE(ycbcr_itu_bt_601_1_rgb);
        COLORCASE(ycbcr_itu_bt_601_1_video);
        COLORCASE(grayscale);
        COLORCASE(photo_ycc);
        COLORCASE(rgb);
        COLORCASE(cmy);
        COLORCASE(cmyk);
        COLORCASE(ycck);
        COLORCASE(cie_lab);
        COLORCASE(bi_level_white);
    }
    return nullptr;
}
static const char* spiff_profile_id_to_string(const charls::spiff_profile_id& profile_id)
{
    switch (profile_id)
    {
        PROFILECASE(none);
        PROFILECASE(continuous_tone_base);
        PROFILECASE(continuous_tone_progressive);
        PROFILECASE(bi_level_facsimile);
        PROFILECASE(continuous_tone_facsimile);
    }
    return nullptr;
}

static const char* spiff_compression_type_to_string(const charls::spiff_compression_type& compression_type)
{
    switch (compression_type)
    {
        COMPCASE(uncompressed);
        COMPCASE(modified_huffman);
        COMPCASE(modified_read);
        COMPCASE(modified_modified_read);
        COMPCASE(jbig);
        COMPCASE(jpeg);
        COMPCASE(jpeg_ls);
    }
    return nullptr;
}
static const char* spiff_resolution_units_to_string(const charls::spiff_resolution_units& resolution_units)
{
    switch (resolution_units)
    {
        RESCASE(aspect_ratio);
        RESCASE(dots_per_inch);
        RESCASE(dots_per_centimeter);
    }
    return nullptr;
}
static const char* interleave_mode_to_string(const charls::interleave_mode& interleave_mode)
{
    switch (interleave_mode)
    {
        MODECASE(none);
        MODECASE(line);
        MODECASE(sample);
    }
    return nullptr;
}
static const char* color_transformation_to_string(const charls::color_transformation& color_transformation)
{
    switch (color_transformation)
    {
        COLTRANCASE(none);
        COLTRANCASE(hp1);
        COLTRANCASE(hp2);
        COLTRANCASE(hp3);
    }
    return nullptr;
}


#undef COLTRANCASE
#undef MODECASE
#undef RESCASE
#undef COMPCASE
#undef PROFILECASE
#undef COLORCASE
#undef TYPECASE


#define OSTREAMOP(TYPE) \
    static std::ostream& operator<<(std::ostream& os, const charls::TYPE& type) \
    { \
        const char* str = TYPE##_to_string(type); \
        os << (str ? str : "unhandled " #TYPE); \
        return os; \
    }

OSTREAMOP(spiff_color_space)
OSTREAMOP(spiff_profile_id)
OSTREAMOP(spiff_compression_type)
OSTREAMOP(spiff_resolution_units)
OSTREAMOP(interleave_mode)
OSTREAMOP(color_transformation)

#undef OSTREAMOP


#define PRINT(S, K) \
    writer.print_value(os, #K, S.K); \
    writer.print_value_separator(os, false);

#define PRINTONLY(S, K) \
    writer.print_value(os, #K, S.K); \
    writer.print_value_separator(os, true);

void print_spiff_header(writer& writer, charls::spiff_header const& spiff_header)
{
    std::ostream& os = std::cout;
    const char header[] = "spiff_header";
    writer.print_header(os, header);
    PRINT(spiff_header, profile_id);          // P: Application profile, type I.8
    PRINT(spiff_header, component_count);     // NC: Number of color components, range [1, 255], type I.8
    PRINT(spiff_header, height);              // HEIGHT: Number of lines in image, range [1, 4294967295], type I.32
    PRINT(spiff_header, width);               // WIDTH: Number of samples per line, range [1, 4294967295], type I.32
    PRINT(spiff_header, color_space);         // S: Color space used by image data, type is I.8
    PRINT(spiff_header, bits_per_sample);     // BPS: Number of bits per sample, range (1, 2, 4, 8, 12, 16), type is I.8
    PRINT(spiff_header, compression_type);    // C: Type of data compression used, type is I.8
    PRINT(spiff_header, resolution_units);    // R: Type of resolution units, type is I.8
    PRINT(spiff_header, vertical_resolution); // VRES: Vertical resolution, range [1, 4294967295], type can be F or I.32
    PRINTONLY(spiff_header,
              horizontal_resolution); // HRES: Horizontal resolution, range [1, 4294967295], type can be F or I.32
    writer.print_footer(os, header);
}

void print_preset_coding_parameters(writer& writer, charls::jpegls_decoder const& decoder)
{
    std::ostream& os = std::cout;
    const char header[] = "preset_coding_parameters";
    writer.print_header(os, header);
    const charls::jpegls_pc_parameters& pcp = decoder.preset_coding_parameters();
    PRINT(pcp, maximum_sample_value);
    PRINT(pcp, threshold1);
    PRINT(pcp, threshold2);
    PRINT(pcp, threshold3);
    PRINTONLY(pcp, reset_value);
    writer.print_footer(os, header);
}
void print_frame_info(writer& writer, charls::jpegls_decoder const& decoder)
{
    std::ostream& os = std::cout;
    const char header[] = "frame_info";
    writer.print_header(os, header);
    const charls::frame_info& frame_info = decoder.frame_info();
    PRINT(frame_info, width);
    PRINT(frame_info, height);
    PRINT(frame_info, bits_per_sample);
    PRINTONLY(frame_info, component_count);
    writer.print_footer(os, header);
}
static void print_hash(writer& writer, charls::jpegls_decoder const& decoder)
{
    std::vector<uint8_t> decoded_buffer(decoder.destination_size());
    decoder.decode(decoded_buffer);
    std::string crc32 = get_crc32(decoded_buffer);
    std::ostream& os = std::cout;
    const char header[] = "hash";
    writer.print_header(os, header);
    writer.print_value(os, "crc32", crc32);
    writer.print_value_separator(os, true);
    writer.print_footer(os, header);
}

#undef PRINT
#undef PRINTONLY


#define PRINTFUN(S, K) \
    writer.print_value(os, #K, S.K()); \
    writer.print_value_separator(os, false);
#define PRINTFUNONLY(S, K) \
    writer.print_value(os, #K, S.K()); \
    writer.print_value_separator(os, true);


void print_header(writer& writer, charls::jpegls_decoder const& decoder)
{
    std::ostream& os = std::cout;
    const char header[] = "header";
    writer.print_header(os, header);
    print_frame_info(writer, decoder);
    writer.print_value_separator(std::cout, false);
    PRINTFUN(decoder, near_lossless);
    PRINTFUN(decoder, interleave_mode);
    print_preset_coding_parameters(writer, decoder);
    writer.print_value_separator(std::cout, false);
    PRINTFUNONLY(decoder, color_transformation);
    writer.print_footer(os, header);
}
#undef PRINTFUN

static void dump(writer& writer, std::string const& filename, bool with_hash)
{
    const std::vector<uint8_t> encoded_source = read_file(filename);

    charls::jpegls_decoder decoder;
    decoder.source(encoded_source);
    // start decoding to check any exception:
    decoder.read_spiff_header();

    writer.print_header(std::cout, "");
    if (decoder.spiff_header_has_value())
    {
        print_spiff_header(writer, decoder.spiff_header());
        writer.print_value_separator(std::cout, false);
    }
    decoder.read_header();
    print_header(writer, decoder);

    if (with_hash)
    {
        writer.print_value_separator(std::cout, false);
        print_hash(writer, decoder);
    }
    writer.print_value_separator(std::cout, true);

    writer.print_footer(std::cout, "");
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    jlst::info_options options{};
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
        const bool multiple = options.inputs.size() > 1;
        for (auto& filename : options.inputs)
        {
            if (multiple)
                std::cout << filename << ":" << std::endl;
            if (options.format == "yaml")
            {
                yaml_writer writer(options.pretty);
                dump(writer, filename, options.with_hash);
            }
            else if (options.format == "json")
            {
                json_writer writer(options.pretty);
                dump(writer, filename, options.with_hash);
            }
            else if (options.format == "xml")
            {
                xml_writer writer(options.pretty);
                dump(writer, filename, options.with_hash);
            }
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


    return EXIT_SUCCESS;
}
