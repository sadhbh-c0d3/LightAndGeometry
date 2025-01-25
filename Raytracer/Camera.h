#ifndef INCLUDED_CAMERA_H
#define INCLUDED_CAMERA_H

#include <algorithm>


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

        struct TraceLineInfo
        {
            int lineNo;
            int ynEnd;
            int pitch;
            NumericType yDelta;
            RayType ray;
            char* ynPixels;
        };

        template<class PixelType>
            void calculateTraceLines(int threadNum, int numThreads, TargetBuffer<PixelType>& target, std::vector<TraceLineInfo>& traceLines)
            {
                TraceLineInfo traceLine{};

                traceLine.ynEnd = target.height;

                traceLine.pitch = target.pitch;

                traceLine.yDelta = (mFrustum.mTop - mFrustum.mBottom) / NumericType(traceLine.ynEnd - 1);

                traceLine.ray.direction[0] = mFrustum.mLeft;
                traceLine.ray.direction[1] = mFrustum.mBottom;
                traceLine.ray.direction[2] = mFrustum.mNear;

                traceLine.ynPixels = target.pixels;

                int linesToDo = traceLine.ynEnd / numThreads;
                int lines = traceLine.ynEnd - (linesToDo * numThreads);
                if (threadNum < lines) {
                    ++linesToDo;
                }

                traceLine.ray.direction[1] += threadNum * traceLine.yDelta;
                traceLine.ynPixels += threadNum * traceLine.pitch;

                for (int lineNo = 0; lineNo < linesToDo; ++lineNo)
                {
                    traceLine.lineNo = lineNo;

                    traceLines.push_back(traceLine);

                    traceLine.ray.direction[1] += numThreads * traceLine.yDelta;
                    traceLine.ynPixels += numThreads * traceLine.pitch;
                }
            }

        void doReorderTraceLines(std::vector<TraceLineInfo>& traceLines, int start, int step)
        {
            const int count = (traceLines.size() - start) / step;

            for (int i = 0; i < count; ++i)
            {
                std::swap(traceLines[start + i], traceLines[start + i * step]);
            }

            if (1 < step)
            {
                doReorderTraceLines(traceLines, count, step / 2);
            }
        }

        void reorderTraceLines(std::vector<TraceLineInfo>& traceLines)
        {
            doReorderTraceLines(traceLines, 0, 4);
        }

        template<class PixelType>
            void raytraceLines(
                    SceneGraphType &sceneGraph,
                    TargetBuffer<PixelType>& target,
                    NumericType brightness,
                    bool disableFSAA,
                    std::vector<TraceLineInfo> &traceLines, int &stop)
            {
                const int xnEnd = target.width;
                const NumericType xDelta = (mFrustum.mRight - mFrustum.mLeft) / NumericType(xnEnd - 1);

                std::vector<TraceLineInfo>::const_iterator iter = traceLines.begin();
                std::vector<TraceLineInfo>::const_iterator end = traceLines.end();

                for (; iter != end && !stop; ++iter)
                {
                    const NumericType yDelta = (*iter).yDelta;

                    RayType ray = (*iter).ray;
                    char* xnPixels = (*iter).ynPixels;

                    for (int xn = 0; xn != xnEnd; ++xn)
                    {
                        ColorType c;

                        if (mFSAA && !disableFSAA)
                        {
                            c = doFSAA(sceneGraph, ray, xDelta, yDelta);
                        }
                        else {
                            c = raytrace(sceneGraph, ray);
                        
                        }

                        c *= brightness;

                        PixelType::putPixel(xnPixels, c);

                        ray.direction[0] += xDelta;
                        xnPixels += PixelType::BytesPerPel;
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