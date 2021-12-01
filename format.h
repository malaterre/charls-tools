// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "cjpls_options.h"
#include "djpls_options.h"
#include <charls/charls.h>

namespace jlst {
class format
{
public:
    virtual ~format(){};
    virtual bool detect(cjpls_options const& options) = 0;
    virtual bool detect2(djpls_options const& options) = 0;
    virtual void open(const char* filename, bool b) = 0;
    virtual void close() = 0;
    // FIXME: for now simple typedef, missing color space
    using info = charls::frame_info;
    using mode = charls::interleave_mode;
    using stride = std::size_t;
    virtual void read_info() = 0;
    virtual void write_info() = 0;
    info& get_info()
    {
        return info_;
    }
    mode& get_mode()
    {
        return mode_;
    }
    stride& get_stride()
    {
        return stride_;
    }
    virtual void read_data(void* buffer, size_t len) = 0;
    virtual void write_data(void* buffer, size_t len) = 0;

protected:
    std::string comment{};
    info info_{};
    mode mode_{};
    stride stride_{};
};
} // namespace jlst
