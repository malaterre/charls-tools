// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#include "format.h"

#include <charls/charls.h>

namespace jlst {
class jls : public format
{
public:
    format* clone() const override;
    bool handle_type(std::string const& type) const override;
    bool detect(source& s, image_info const& ii) const override;

    void read_info(source& s, image& i) const override;
    void read_data(source& s, image& i) const override;

    void write_info(dest& d, const image& i, const jls_options& jo) const override;
    void write_data(dest& d, const image& i, const jls_options& jo) const override;

private:
    charls::jpegls_decoder decoder_;
    charls::jpegls_encoder encoder_;
};
} // namespace jlst
