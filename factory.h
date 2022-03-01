// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <set>
#include <string>

namespace jlst {
class format;
class source;
class factory
{
public:
    static factory& instance();
    bool register_format(const format* f, float priority = 0.5);
    format* get_format_from_type(std::string const& type) const;
    format* detect_format(source& s) const;

private:
    struct entry
    {
        const format* format_;
        float priority_;
        bool operator<(const entry& rhs) const
        {
            return priority_ > rhs.priority_;
        }
    };
    std::multiset<entry> formats;
};
} // end namespace jlst
