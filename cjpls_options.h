// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <charls/charls.h>
#include <string>

namespace jlst {
struct cjpls_options final
{
    std::string input{};
    std::string output{};
    charls::frame_info frame_info{};
    charls::interleave_mode interleave_mode{};
    int near_lossless{};
    charls::jpegls_pc_parameters preset_coding_parameters{};
    charls::color_transformation color_transformation{};
    bool standard_spiff_header{};
    charls::interleave_mode planar_configuration{};

    /**
     * Returns false when the process should stop, ie `help` or `version` was passed.
     * Returns true when the next step encode/decode should continue.
     */
    bool process(int argc, char* argv[]);
};
} // namespace jlst
