// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string>

namespace jlst {
class source;
class dest;
class image;
class image_info;
struct jls_options;
class format
{
public:
    virtual ~format()
    {
    }
    virtual format* clone() const = 0;
    virtual bool handle_type(std::string const& type) const = 0;
    virtual bool detect(source& s, image_info const& ii) const = 0;

    image load(source& s, image_info const& ii) const;
    void save(dest& d, image const& i, jls_options const& options) const;

protected:
    virtual void read_info(source& s, image& i) const = 0;
    virtual void read_data(source& s, image& i) const = 0;

    virtual void write_info(dest& d, const image& i, const jls_options& jo) const = 0;
    virtual void write_data(dest& d, const image& i, const jls_options& jo) const = 0;
};
} // namespace jlst
