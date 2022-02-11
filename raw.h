// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#include "format.h"

#include <fstream>

namespace jlst {
class raw : public format
{
public:
    static const format& get();
    bool detect(source& s, image_info const& ii) const override;
    bool detect2(djpls_options const& options) const override;

    void read_info(source& s, image& i) const override;
    void read_data(source& s, image& i) const override;

    void write_info(dest& d, const image& i, const jls_options& jo) const override;
    void write_data(dest& d, const image& i, const jls_options& jo) const override;
};
} // namespace jlst
