// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace jlst {
class source
{
public:
    source();
    source(std::string const& filename);
    ~source();

    int peek();
    void rewind();
    size_t read(void* ptr, size_t n);
    std::string getline();

    const std::string& get_filename() const
    {
        return filename_;
    }
    std::vector<uint8_t> read_bytes();

    source(source&& s)
    {
        stream_ = s.stream_;
        filename_ = s.filename_;
        s.stream_ = nullptr;
        s.filename_ = "";
    }

private:
    source(const source& s);            // Copy constructor
    source& operator=(const source& s); // Assignment operator
    source& operator=(source&& s);      // move assignement

    FILE* stream_;
    std::string filename_;
};

} // namespace jlst
