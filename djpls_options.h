// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "options.h"

#include <charls/charls.h>
#include <string>

namespace jlst {
struct djpls_options final : options
{
    std::string type_;
    const std::string& get_type() const
    {
        return type_;
    }

    charls::interleave_mode planar_configuration{};

    /**
     * Returns false when the process should stop, ie `help` or `version` was passed.
     * Returns true when the next step encode/decode should continue.
     */
    bool process(int argc, char* argv[]);
};
} // namespace jlst
