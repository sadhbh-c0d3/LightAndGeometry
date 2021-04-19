#ifndef INCLUDED_TARGET_BUFFER_H
#define INCLUDED_TARGET_BUFFER_H

#include "Linear.h"
#include "MinMax.h"

struct PixelRGBA32
{
    enum {BytesPerPel = 4};

    template<class NumericType>
        static int clamp(const NumericType &value)
        {
            return Max(0, Min(255, (int)(value * 255.0)));
        }

    template<class NumericType>
        static void putPixel(char *position, const GAL_imp::Point<NumericType,4> &value)
        {
            position[0] = clamp(value[0]);
            position[1] = clamp(value[1]);
            position[2] = clamp(value[2]);
            position[3] = clamp(value[3]);
        }
};

template<class _PixelType>
    struct TargetBuffer
    {
        typedef _PixelType PixelType;

        char      *pixels;
        int        pitch;
        int        width;
        int        height;

        TargetBuffer(): pixels(NULL)
        {
        }
    };


#endif