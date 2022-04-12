// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "image.h"
#include "options.h"

#include <charls/charls.h>

namespace jlst {
struct jls_options final
{
    bool has_interleave_mode{};
    charls::interleave_mode interleave_mode{};
    int near_lossless{};
    charls::jpegls_pc_parameters preset_coding_parameters{};
    charls::color_transformation color_transformation{};
    bool standard_spiff_header{true};
#if CHARLS_VERSION_MAJOR > 2 || (CHARLS_VERSION_MAJOR == 2 && CHARLS_VERSION_MINOR > 2)
    charls::encoding_options encoding_options{};
#endif
};
struct cjpls_options final : options
{
    std::string type_;
    const std::string& get_type() const
    {
        return type_;
    }
    // options for input image (raw input)
    image_info image_info_;
    const image_info& get_image_info() const
    {
        return image_info_;
    }

    // output options for JPEG-LS bitstream:
    jls_options jls_options_;
    const jls_options& get_jls_options() const
    {
        return jls_options_;
    }
    /**
     * Returns false when the process should stop, ie `help` or `version` was passed.
     * Returns true when the next step encode/decode should continue.
     */
    bool process(int argc, char* argv[]);
};
} // namespace jlst
