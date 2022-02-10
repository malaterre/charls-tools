// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#include "format.h"

#include <fstream>

namespace jlst {
class pnm : public format
{
public:
    static const format& get();
    // bool detect(cjpls_options const& options) override;
    bool detect(image_info const& ii, source& s) const override;
    bool detect2(djpls_options const& options) const override;

    void read_info(source& s, image& i) const override;
    void write_info(dest& d, const image& i) const override;
    void read_data(source& s, image& i) const override;
    void write_data(dest& d, const image& i) const override;
};
} // namespace jlst
