// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#include "format.h"

#include <fstream>

namespace jlst {
class raw : public format
{
public:
    static format& get();
    bool detect(source& s) override;
    bool detect2(djpls_options const& options) override;
    //    void open(const char* filename, bool b) override;
    //    void close() override;
    void read_info(source& s) override;
    void write_info(dest& d) override;
    void read_data(source& s, void* buffer, size_t len) override;
    void write_data(dest& d, const void* buffer, size_t len) override;

private:
    std::ifstream ifs{};
};
} // namespace jlst
