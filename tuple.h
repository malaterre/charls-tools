// Copyright (c) Mathieu Malaterre
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <iostream>

// boost::program_options does not handle std::array so write our own wrapper
// https://stackoverflow.com/questions/38465195/boostprogram-options-appears-to-cast-stdarray-to-boostarray
template<typename T, int N>
class tuple;
template<typename T, int N>
std::istream& operator>>(std::istream& is, tuple<T, N>& rhs);
template<typename T, int N>
std::ostream& operator<<(std::ostream& os, const tuple<T, N>& rhs);
template<typename T, int N>
struct tuple
{
    T values[N];
    friend std::istream& operator>><>(std::istream& is, tuple<T, N>& val);
    friend std::ostream& operator<<<>(std::ostream& os, const tuple<T, N>& val);
};
template<typename T, int N>
inline std::istream& operator>>(std::istream& is, tuple<T, N>& val)
{
    for (int i = 0; i < N; ++i)
    {
        if (i)
        {
            if (is.peek() == ',')
            {
                is.ignore();
            }
        }
        is >> val.values[i];
    }
    return is;
}
template<typename T, int N>
inline std::ostream& operator<<(std::ostream& os, const tuple<T, N>& val)
{
    for (int i = 0; i < N; ++i)
    {
        if (i)
        {
            os << ',';
        }
        os << val.values[i];
    }
    return os;
}
