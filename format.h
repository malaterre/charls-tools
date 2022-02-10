// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "cjpls_options.h"
#include "djpls_options.h"
#include <charls/charls.h>

namespace jlst {
class source;
class dest;
class image;
class format
{
public:
    virtual ~format(){};
    // virtual bool detect(cjpls_options const& options) = 0;
    virtual bool detect(image_info const& ii, source& s) const = 0;
    virtual bool detect2(djpls_options const& options) const = 0;

    virtual void read_info(source& s, image& i) const = 0;
    virtual void write_info(dest& d, const image& i) const = 0;
    virtual void read_data(source& s, image& i) const = 0;
    virtual void write_data(dest& d, const image& i) const = 0;
};
} // namespace jlst
