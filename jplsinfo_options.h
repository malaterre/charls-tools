// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "options.h"

#include <string>

namespace jlst {
struct info_options final : options
{
    info_options() : format("json")
    {
    }
    std::string format{};
    bool pretty{};
    bool with_hash{};

    /**
     * Returns false when the process should stop, ie `help` or `version` was passed.
     * Returns true when the next step encode/decode should continue.
     */
    bool process(int argc, char* argv[]);
};
} // namespace jlst
