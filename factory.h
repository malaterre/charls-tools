#pragma once

namespace jlst {
class factory
{
public:
    static factory& instance();
    void registerCodec();
};
} // end namespace jlst
