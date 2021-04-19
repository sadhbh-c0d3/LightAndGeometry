#ifndef INCLUDED_FRUSTUM_H
#define INCLUDED_FRUSTUM_H


template<class _NumericType>
    struct Frustum
    {
        typedef _NumericType NumericType;

        NumericType mLeft;
        NumericType mRight;
        NumericType mBottom;
        NumericType mTop;
        NumericType mNear;
        NumericType mFar;
    };


typedef Frustum<float> Frustum3f;
typedef Frustum<double> Frustum3d;

#endif