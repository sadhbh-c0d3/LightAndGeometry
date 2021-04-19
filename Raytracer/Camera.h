#ifndef INCLUDED_CAMERA_H
#define INCLUDED_CAMERA_H

template<class _NumericType>
    class Camera
    {
    public:
        typedef _NumericType                        NumericType;
        typedef GAL_imp::Point<NumericType,3>       PointType;
        typedef GAL_imp::Point<NumericType,4>       ColorType;
        typedef GAL_imp::Matrix<NumericType,3>      TransformType;
        typedef GAL_imp::Ray<NumericType,3>         RayType;
        typedef IntersectionPoint<NumericType,3>    IntersectionPointType;
        typedef SceneGraph<NumericType>             SceneGraphType;
        typedef Frustum<NumericType>                FrustumType;

        Camera(): mFlags(0), mFSAA(false), mRecursionDepth(0)
        {
            mLTM.Row(0) = GAL_imp::P3_<NumericType>(1,0,0);
            mLTM.Row(1) = GAL_imp::P3_<NumericType>(0,1,0);
            mLTM.Row(2) = GAL_imp::P3_<NumericType>(0,0,1);
        }

        void setTranslation(const PointType &translation)
        {
            mTranslation = translation;
        }

        void setLocalTransform(const TransformType &ltm, unsigned long flags)
        {
            mLTM = ltm;
            mFlags = flags;
        }

        void setFSAA(bool val)
        {
            mFSAA = val;
        }

        bool isFSAA()
        {
            return mFSAA;
        }

        void setRecursionDepth(int val)
        {
            mRecursionDepth = val;
        }

        int getRecursionDepth()
        {
            return mRecursionDepth;
        }

        void setFrustum(NumericType iLeft, NumericType iRight,
                        NumericType iBottom, NumericType iTop,
                        NumericType iNear, NumericType iFar)
        {
            mFrustum.mLeft = iLeft;
            mFrustum.mRight = iRight;
            mFrustum.mBottom = iBottom;
            mFrustum.mTop = iTop;
            mFrustum.mNear = iNear;
            mFrustum.mFar = iFar;
        }

        FrustumType & getFrustum()
        {
            return mFrustum;
        }

        const FrustumType & getFrustum() const
        {
            return mFrustum;
        }

        ColorType raytrace(SceneGraphType &sceneGraph, RayType ray)
        {
            //
            // Transform ray to world coordinates
            //
            ray.start = (mLTM * ray.start) + mTranslation;
            ray.direction = mLTM * ray.direction;

            return sceneGraph.raytrace(ray, mRecursionDepth);
        }

        ColorType doFSAA(SceneGraphType &sceneGraph, const RayType &ray, NumericType xDelta, NumericType yDelta)
        {
            ColorType c;

            for (int iy = 0; iy < 4; ++iy)
            {
                for (int ix = 0; ix < 4; ++ix)
                {
                    double a = (double(ix) - 2.5) * xDelta * 0.2;
                    double b = (double(iy) - 2.5) * yDelta * 0.2;

                    RayType aaray;
                    aaray.start = ray.start;
                    aaray.direction[0] = ray.direction[0] + a;
                    aaray.direction[1] = ray.direction[1] + b;
                    aaray.direction[2] = ray.direction[2];

                    c += raytrace(sceneGraph, aaray);
                }
            }
            return c / 16;
        }

        template<class PixelType>
            void raytrace(int threadNum, int numThreads, int &stop, SceneGraphType &sceneGraph, TargetBuffer<PixelType> &target)
            {
                const int pitch = target.pitch;
                const int xnEnd = target.width;
                const int ynEnd = target.height;

                const NumericType xDelta = (mFrustum.mRight - mFrustum.mLeft) / NumericType(xnEnd - 1);
                const NumericType yDelta = (mFrustum.mTop - mFrustum.mBottom) / NumericType(ynEnd - 1);

                RayType ray;
                ray.direction[1] = mFrustum.mBottom;
                ray.direction[2] = mFrustum.mNear;

                char *ynPixels = target.pixels;

                int linesToDo = ynEnd / numThreads;
                int lines = ynEnd - (linesToDo * numThreads);
                if (threadNum < lines) {
                    ++linesToDo;
                }

                for (int n = 0; n < threadNum; ++n)
                {
                    ray.direction[1] += yDelta;
                    ynPixels += pitch;
                }

                for (int lineNo = 0; lineNo < linesToDo && !stop; ++lineNo)
                {
                    ray.direction[0] = mFrustum.mLeft;
                    char *xnPixels = ynPixels;

                    for (int xn = 0; xn != xnEnd; ++xn)
                    {
                        ColorType c;

                        if (mFSAA)
                        {
                            c = doFSAA(sceneGraph, ray, xDelta, yDelta);
                        }
                        else {
                            c = raytrace(sceneGraph, ray);
                        }

                        PixelType::putPixel(xnPixels, c);

                        ray.direction[0] += xDelta;
                        xnPixels += PixelType::BytesPerPel;
                    }

                    for (int n = 0; n < numThreads; ++n)
                    {
                        ray.direction[1] += yDelta;
                        ynPixels += pitch;
                    }
                }
            }

    private:
        PointType       mTranslation;
        TransformType   mLTM;
        unsigned long   mFlags;
        FrustumType     mFrustum;
        bool            mFSAA;
        int             mRecursionDepth;
    };

typedef Camera<float> Camera3f;
typedef Camera<double> Camera3d;


#endif