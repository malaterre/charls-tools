// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cstdio>
#include <string>

namespace jlst {
class dest
{
public:
    dest();
    dest(std::string const& filename);
    ~dest();

    size_t write(const void* ptr, size_t n);

    dest(dest&& s)
    {
        stream_ = s.stream_;
        filename_ = s.filename_;
        s.stream_ = nullptr;
        s.filename_ = "";
    }

private:
    dest(const dest& s);            // Copy constructor
    dest& operator=(const dest& s); // Assignment operator
    dest& operator=(dest&& s);      // move assignement

    FILE* stream_;
    std::string filename_;
};

} // namespace jlst
