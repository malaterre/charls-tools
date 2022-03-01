// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "jls.h"

#include "cjpls_options.h"
#include "factory.h"
#include "image.h"
#include "jplstran_options.h"
#include "utils.h"

#include <cassert>
#include <sstream>
#include <vector>

namespace jlst {
bool jls::handle_type(std::string const& type) const
{
    return type == "jls";
}
bool jls::detect(source& s, image_info const&) const
{
    try
    {
        charls::jpegls_decoder decoder;
        std::vector<uint8_t> buf;
        buf.resize(64);
        s.read(buf.data(), buf.size());
        decoder.source(buf);
        decoder.read_header();
        return true;
    }
    catch (std::exception& e)
    {
    }
    return false;
}

void jls::read_info(source& fs, image& i) const
{
    fs.rewind();
    charls::jpegls_decoder decoder;
    std::vector<uint8_t> buf;
    buf.resize(64);
    fs.read(buf.data(), buf.size());
    decoder.source(buf);
    // comment handling, must be setup before any read_* function
    std::string comment;
#if CHARLS_VERSION_MAJOR > 2 || (CHARLS_VERSION_MAJOR == 2 && CHARLS_VERSION_MINOR > 2)
    {
        decoder.at_comment([&comment](const void* data, const size_t size) noexcept {
            comment = std::string(static_cast<const char*>(data), size);
        });
    }
#endif

    decoder.read_header();
    i.get_image_info().frame_info() = decoder.frame_info();
    i.get_image_info().interleave_mode() = decoder.interleave_mode();
    i.get_image_info().comment() = comment;
}

namespace {
static void decompress(charls::jpegls_decoder& decoder, source& fs, image& i)
{
    const std::vector<uint8_t> encoded_source = fs.read_bytes();
    decoder.source(encoded_source);
    // comment handling, must be setup before any read_* function
    std::string comment;
#if CHARLS_VERSION_MAJOR > 2 || (CHARLS_VERSION_MAJOR == 2 && CHARLS_VERSION_MINOR > 2)
    {
        decoder.at_comment([&comment](const void* data, const size_t size) noexcept {
            comment = std::string(static_cast<const char*>(data), size);
        });
    }
#endif
    decoder.read_header();

    i.get_image_info().frame_info() = decoder.frame_info();
    i.get_image_info().interleave_mode() = decoder.interleave_mode();
    i.get_image_info().comment() = comment;

    // std::vector<uint8_t> decoded_buffer(decoder.destination_size());
    auto& decoded_buffer = i.get_image_data().pixel_data();
    decoded_buffer.resize(decoder.destination_size());
    decoder.decode(decoded_buffer);
}
} // end namespace

void jls::read_data(source& fs, image& i) const
{
    fs.rewind();
    charls::jpegls_decoder decoder;
    decompress(decoder, fs, i);
}

namespace {
static std::vector<uint8_t> compress(image const& img, const jlst::jls_options& options)
{
    auto const& frame_info = img.get_image_info().frame_info();
    // what if user requested 'line' or 'sample' for single component ? Let's
    // handle it here (not sure why charls does not handle it internally).
    // `jpeg` seems to handle line/sample for single input...not clear what is legal
    if (frame_info.component_count == 1)
    {
        if (options.interleave_mode != charls::interleave_mode::none)
            throw std::invalid_argument("Invalid interleave_mode for single component. Use 'none'");
    }

    charls::jpegls_encoder encoder;

    // setup encoder using compressor's options:
    encoder
        .interleave_mode(options.interleave_mode)                   // interleave_mode
        .near_lossless(options.near_lossless)                       // near_lossless
        .preset_coding_parameters(options.preset_coding_parameters) // preset_coding_parameters
        .color_transformation(options.color_transformation);        // color_transformation

#if CHARLS_VERSION_MAJOR > 2 || (CHARLS_VERSION_MAJOR == 2 && CHARLS_VERSION_MINOR > 2)
    {
        encoder.encoding_options(options.encoding_options);
    }
    {
        // the following writes an extra \0
        // encoder.write_comment(image.get_image_info().comment().c_str());
        auto& comment = img.get_image_info().comment();
        if (!comment.empty())
            encoder.write_comment(comment.c_str(), comment.size());
    }
#endif

    // setup encoder using input image:
    encoder.frame_info(frame_info); // frame_info

    // allocate output:
    std::vector<uint8_t> buffer(encoder.estimated_destination_size());
    encoder.destination(buffer);
    // now that destination buffer is set, write SPIFF header:
    if (options.standard_spiff_header)
    {
        const charls::spiff_color_space spiff_color_space =
            frame_info.component_count == 1 ? charls::spiff_color_space::grayscale : charls::spiff_color_space::rgb;
        encoder.write_standard_spiff_header(spiff_color_space);
    }

    const auto transform_pixel_data{img.transform(options.interleave_mode)};
    size_t encoded_size;
    if (options.interleave_mode == charls::interleave_mode::none)
    {
        encoded_size = encoder.encode(transform_pixel_data);
    }
    else
    {
        encoded_size = encoder.encode(transform_pixel_data, img.get_image_data().stride());
    }
    buffer.resize(encoded_size);

    return buffer;
}
} // end namespace

void jls::write_info(dest&, const image&, const jls_options&) const
{
}

void jls::write_data(dest& fs, const image& i, const jls_options& jo) const
{
    auto encoded_buffer{compress(i, jo)};
    fs.write(encoded_buffer.data(), encoded_buffer.size());
}

namespace {
static void patch_header(std::vector<uint8_t>& v, int near)
{
    size_t pos = 0;
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        if (*it == 0xff)
        {
            ++it;
            if (it != v.end() && *it == 0xda)
            {
                ++it;
                pos = it - v.begin();
            }
        }
    }
    if (pos == 0)
        throw std::runtime_error("cannot find scan header");
    // Ls:
    uint16_t ls = v[pos + 0] << 8 | v[pos + 1];

    v[pos + 9] = near;
}
} // end namespace

void jls::transform(dest& d, source& s, const tran_options& to) const
{
    jlst::image input_image;
    charls::jpegls_decoder decoder;
    decompress(decoder, s, input_image);

    jls_options jo{};
    jo.interleave_mode = decoder.interleave_mode();
    jo.near_lossless = 0; // important
    jo.preset_coding_parameters = decoder.preset_coding_parameters();
    jo.color_transformation = decoder.color_transformation();
    jo.standard_spiff_header = decoder.spiff_header_has_value();

    jlst::image output_image;
    if (to.type == tran_options::transform_type::crop)
    {
        output_image.get_image_info() = input_image.get_image_info();
        auto& region = to.region;
        output_image.get_image_info().frame_info().width = region.Width;
        output_image.get_image_info().frame_info().height = region.Height;
        output_image.get_image_data().pixel_data() = input_image.crop(region.X, region.Y, region.Width, region.Height);
    }
    else if (to.type == tran_options::transform_type::flip)
    {
        output_image.get_image_info() = input_image.get_image_info();
        output_image.get_image_data().pixel_data() = input_image.flip(to.vertical);
    }
    else if (to.type == tran_options::transform_type::rotate)
    {
        output_image.get_image_info() = input_image.get_image_info();
        if (to.degree == 90 || to.degree == 270)
        {
            output_image.get_image_info().frame_info().width = input_image.get_image_info().frame_info().height;
            output_image.get_image_info().frame_info().height = input_image.get_image_info().frame_info().width;
        }
        output_image.get_image_data().pixel_data() = input_image.rotate(to.degree);
    }
    else if (to.type == tran_options::transform_type::transpose)
    {
        output_image.get_image_info() = input_image.get_image_info();
        output_image.get_image_info().frame_info().width = input_image.get_image_info().frame_info().height;
        output_image.get_image_info().frame_info().height = input_image.get_image_info().frame_info().width;

        output_image.get_image_data().pixel_data() = input_image.transpose();
    }
    else if (to.type == tran_options::transform_type::transverse)
    {
        output_image.get_image_info() = input_image.get_image_info();
        output_image.get_image_info().frame_info().width = input_image.get_image_info().frame_info().height;
        output_image.get_image_info().frame_info().height = input_image.get_image_info().frame_info().width;

        output_image.get_image_data().pixel_data() = input_image.transverse();
    }
    else if (to.type == tran_options::transform_type::wipe)
    {
        output_image.get_image_info() = input_image.get_image_info();
        auto& region = to.region;
        output_image.get_image_data().pixel_data() = input_image.wipe(region.X, region.Y, region.Width, region.Height);
    }
    else
    {
        throw std::runtime_error("wotsit");
    }
    auto encoded_buffer{compress(output_image, jo)};
    //    patch_header(encoded_buffer, decoder.near_lossless());
    d.write(encoded_buffer.data(), encoded_buffer.size());
}

format* jls::clone() const
{
    return new jls;
}

static const format* get()
{
    static const jls jls_;
    return &jls_;
}
static bool b = factory::instance().register_format(get(), 1);
} // namespace jlst
