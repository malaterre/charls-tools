// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "dest.h"
#include "source.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace jlst {
struct options
{
    source& get_source(int index);
    dest& get_dest(int index);

    std::vector<source>& get_sources()
    {
        return sources;
    }

protected:
    void add_inputs(std::vector<std::string> const& inputs)
    {
        for (auto& input : inputs)
        {
            sources.push_back(source(input.c_str()));
        }
    }
    void add_outputs(std::vector<std::string> const& outputs)
    {
        for (auto& output : outputs)
        {
            dests.push_back(dest(output.c_str()));
        }
    }
    void add_stdin_input()
    {
        if (!is_stdin_connected_to_terminal())
        {
            sources.push_back(source());
        }
        else
        {
            throw std::invalid_argument("missing input");
        }
    }
    void add_stdout_output(bool check_stdout = true)
    {
        if (!check_stdout || !is_stdout_connected_to_terminal())
        {
            dests.push_back(dest());
        }
        else
        {
            throw std::invalid_argument("missing output");
        }
    }

private:
    static bool is_stdin_connected_to_terminal();
    static bool is_stdout_connected_to_terminal();
    std::vector<source> sources{};
    std::vector<dest> dests{};
};
} // namespace jlst
