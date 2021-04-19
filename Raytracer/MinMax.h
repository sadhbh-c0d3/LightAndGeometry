#ifndef INCLUDED_MINMAX_H
#define INCLUDED_MINMAX_H

template<class N>
    const N &Min(const N &a, const N &b)
    {
        return (a < b ? a : b);
    }

template<class N>
    const N &Max(const N &a, const N &b)
    {
        return (a > b ? a : b);
    }

#endif