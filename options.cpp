// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#include "options.h"

#include <unistd.h>

namespace jlst {
bool options::is_stdin_connected_to_terminal()
{
#ifdef HAVE_ISATTY
    // No point in checking `errno`, simply ret val:
    return isatty(STDIN_FILENO) == 1;
#else
#error isatty missing
#endif
}
bool options::is_stdout_connected_to_terminal()
{
#ifdef HAVE_ISATTY
    // No point in checking `errno`, simply ret val:
    return isatty(STDOUT_FILENO) == 1;
#else
#error isatty missing
#endif
}

source& options::get_source(int index)
{
    return sources[index];
}

dest& options::get_dest(int index)
{
    return dests[index];
}

} // end namespace jlst
