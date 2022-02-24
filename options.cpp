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
std::string options::compute_type_from_filenames(std::vector<std::string> const& outputs)
{
    if (outputs.empty())
        throw std::runtime_error("compute_type_from_outputs is empty");
    auto& ref = outputs[0];
    auto pos = ref.rfind('.');
    if (pos != std::string::npos)
    {
        return ref.substr(pos + 1);
    }
    throw std::runtime_error("compute_type_from_outputs no file extension");
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
