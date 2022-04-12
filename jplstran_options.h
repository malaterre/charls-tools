// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "options.h"

#include <string>

namespace jlst {
struct tran_options final : options
{
    enum class transform_type
    {
        none = 0,
        crop,
        flip,
        rotate,
        transpose,
        transverse,
        wipe
    };
    transform_type type{};
    int degree{};
    bool vertical{};
    struct
    {
        uint32_t X;
        uint32_t Y;
        uint32_t Width;
        uint32_t Height;
    } region;
    bool jai_imageio{};
    bool standard_spiff_header{};

    /**
     * Returns false when the process should stop, ie `help` or `version` was passed.
     * Returns true when the next step encode/decode should continue.
     */
    bool process(int argc, char* argv[]);
};
} // namespace jlst
