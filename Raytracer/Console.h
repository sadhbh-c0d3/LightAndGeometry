#ifndef INCLUDED_CONSOLE_H
#define INCLUDED_CONSOLE_H

#define MEAN_AND_LEAN
#include <Windows.h>
#include <sstream>
#include "Linear.h"

class Console
{
public:
    static void writeln(const char *msg)
    {
        OutputDebugStringA(msg);
        OutputDebugStringA("\n");
    }

    struct Out : public std::stringstream
    {
        ~Out()
        {
            Console::writeln(str().c_str());
        }

    };

};

template<class N, int I>
std::ostream &operator << (std::ostream &out, const GAL_imp::Point<N,I> &point)
{
    out << point.x[0];

    for (int i = 1; i < I; ++i)
    {
        out << ", " << point.x[i];
    }

    return out;
}

#endif