// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "factory.h"
#include "format.h"
#include "image.h"

namespace jlst {
bool factory::register_format(const format* f, float priority)
{
#if 0
    auto p = std::clamp(priority, 0, 1); // c++17
#else
    auto p = priority < 0 ? 0 : (priority > 1 ? 1 : priority);
#endif
    entry e;
    e.priority_ = p;
    e.format_ = f;
    formats.insert(e);
    return true;
}
format* factory::get_format_from_type(std::string const& type) const
{
    for (auto e : formats)
    {
        auto f = e.format_;
        if (f->handle_type(type))
        {
            return f->clone();
        }
    }

    return nullptr;
}
format* factory::detect_format(source& s) const
{
    image_info ii{};
    for (auto e : formats)
    {
        auto f = e.format_;
        if (f->detect(s, ii))
        {
            return f->clone();
        }
    }

    return nullptr;
}
factory& factory::instance()
{
    static factory factory_;
    return factory_;
}
} // end namespace jlst
