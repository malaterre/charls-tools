#include "factory.h"

namespace jlst {
factory& factory::instance()
{
    static factory factory_;
    return factory_;
}
} // end namespace jlst
