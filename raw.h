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
    bool detect(cjpls_options const& options) override;
    bool detect2(djpls_options const& options) override;
    void open(const char* filename, bool b) override;
    void close() override;
    void read_info() override;
    void write_info() override;
    void read_data(void* buffer, size_t len) override;
    void write_data(void* buffer, size_t len) override;

private:
    std::ifstream ifs{};
};
} // namespace jlst
