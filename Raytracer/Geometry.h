#ifndef INCLUDED_GEOMETRY_H
#define INCLUDED_GEOMETRY_H

#include "Intersect.h"
#include "Mesh.h"
#include "AABBox.h"
#include "BoundingSphere.h"

template<class N, int I>
    struct IntersectionPoint
    {
        GAL_imp::Point<N,I> position;
        GAL_imp::Point<N,I> normal;
        GAL_imp::Point<N,I> tangent;
        GAL_imp::Point<N,4> color;
        N                   distance;
        bool                isReflective;
    };

typedef IntersectionPoint<float,3>  IntersectionPoint3f;
typedef IntersectionPoint<double,3> IntersectionPoint3d;

template<class _NumericType>
    class Geometry
    {
    public:
        typedef _NumericType                        NumericType;
        typedef GAL_imp::Point<NumericType,3>       PointType;
        typedef GAL_imp::Point<NumericType,4>       ColorType;
        typedef GAL_imp::Matrix<NumericType,3>      TransformType;
        typedef GAL_imp::Ray<NumericType,3>         RayType;
        typedef IntersectionPoint<NumericType,3>    IntersectionPointType;

        Geometry(): mFlags(0), mReflective(false)
        {
            mLTM.Row(0) = GAL_imp::P3_<NumericType>(1,0,0);
            mLTM.Row(1) = GAL_imp::P3_<NumericType>(0,1,0);
            mLTM.Row(2) = GAL_imp::P3_<NumericType>(0,0,1);
        }

        bool intersectRay(RayType ray, IntersectionPointType &out)
        {
            // If matrix is orthogonal, then its inverse is simply transposition
            //
            // TODO: mFlags should say whether matrix is or is not orthogonal
            //
            TransformType inverseLTM = mLTM.T();

            // Transform ray to object local coordinates
            ray.start      = inverseLTM * (ray.start - mTranslation);
            ray.direction  = inverseLTM * ray.direction;

            if (!doIntersectRay(ray, out))
            {
                return false;
            }

            // Transform result to original coordinates
            out.position = (mLTM * out.position) + mTranslation;
            out.normal   = mLTM * out.normal;
            out.tangent  = mLTM * out.tangent;

            // Simplified material properties
            out.color = mColor;
            out.isReflective = mReflective;

            return true;
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

        void setColor(const ColorType &color)
        {
            mColor = color;
        }

        void setReflective(bool val)
        {
            mReflective = val;
        }

        bool isReflective()
        {
            return mReflective;
        }

    protected:
        virtual bool doIntersectRay(const RayType &ray, IntersectionPointType &out) = 0;

    private:
        PointType       mTranslation;
        TransformType   mLTM;
        unsigned long   mFlags;
        ColorType       mColor;
        bool            mReflective;
    };

template<class _NumericType>
    class SphereGeometry : public Geometry<_NumericType>
    {
    public:
        SphereGeometry(NumericType radius): mRadius(radius)
        {
        }

    protected:
        bool doIntersectRay(const RayType &ray, IntersectionPointType &out)
        {
            GAL_imp::Solution<NumericType,2> solution;

            if (!GAL::IntersectRaySphere(ray, mRadius, solution))
            {
                return false;
            }

            NumericType t = GAL::ChooseNearestPositiveRoot(solution);
            if (t < 0)
            {
                // sphere is behind
                return false;
            }

            out.distance = t;
            out.position = ray.start + ray.direction * t;
            out.normal = out.position;
            out.tangent = GAL::Orthogonal(out.normal);

            return true;
        }

    private:
        NumericType mRadius;
    };

typedef SphereGeometry<float>  SphereGeometry3f;
typedef SphereGeometry<double> SphereGeometry3d;


template<class _NumericType>
    class CylinderGeometry : public Geometry<_NumericType>
    {
    public:
        CylinderGeometry(NumericType radius, const PointType &height): mRadius(radius), mHeight(height)
        {
        }

    protected:
        bool doIntersectRay(const RayType &ray, IntersectionPointType &out)
        {
            GAL_imp::Solution<NumericType,2> solution;

            if (!GAL::IntersectRayInfiniteCylinder(ray, mRadius, mHeight, solution))
            {
                return false;
            }

            PointType p1 = ray.start + ray.direction * solution.x[0];

            // Check whether intersection point is below cylinder
            if (GAL::Dot(p1, mHeight) < 0)
            {
                PointType p2 = ray.start + ray.direction * solution.x[1];

                if (GAL::Dot(p2, mHeight) < 0)
                {
                    // Both intersection points are below cylinder
                    return false;
                }
                else if (!GAL::IntersectRayPlane(ray, mHeight, out.distance))
                {
                    // Ray is parallel to cylinder bottom
                    return false;
                }
                else if (out.distance < 0)
                {
                    // Plane is behind th ray
                    return false;
                }

                out.position = ray.start + ray.direction * out.distance;
                out.normal   = -mHeight;
                out.tangent  = GAL::ProjectToPlane(out.normal, out.position);
            }
            // Check whether intersectoin point is above cylinder
            else if (0 < GAL::Dot(p1 - mHeight, mHeight))
            {
                PointType p2 = ray.start + ray.direction * solution.x[1];

                if (0 < GAL::Dot(p2 - mHeight, mHeight))
                {
                    // Both intersection points are above cylinder
                    return false;
                }

                RayType ray2;
                ray2.start = ray.start - mHeight;
                ray2.direction = ray.direction;

                if (!GAL::IntersectRayPlane(ray2, mHeight, out.distance))
                {
                    // Ray is parallel to cylinder top
                    return false;
                }
                else if (out.distance < 0)
                {
                    // Plane is behind th ray
                    return false;
                }
                out.position = ray.start + ray.direction * out.distance;
                out.normal = mHeight;
                out.tangent = GAL::ProjectToPlane(out.normal, out.position);
            }
            // Intersection point was on cylinder
            else
            {
                if (solution.x[1] < 0)
                {
                    // Both intersection points are behind ray, thus
                    // cylinder must be behind ray
                    return false;
                }
                else if (solution.x[0] < 0)
                {
                    // First intersection point is behind ray, thus
                    // ray must be inside of cylinder
                    out.distance = solution.x[1];
                    out.position = ray.start + ray.direction * out.distance;
                }
                else
                {
                    // Generic case
                    out.distance = solution.x[0];
                    out.position = p1;
                }

                out.normal = GAL::ProjectToPlane(mHeight, out.position);
                out.tangent = mHeight;
            }
            return true;
        }

    private:
        NumericType mRadius;
        PointType   mHeight;
    };

typedef CylinderGeometry<float>  CylinderGeometry3f;
typedef CylinderGeometry<double> CylinderGeometry3d;


template<class _VertexType, class _IndexType = int>
    class MeshGeometry : public Geometry< typename Mesh<_VertexType, _IndexType>::NumericType >
    {
    public:
        typedef _VertexType                         VertexType;
        typedef _IndexType                          IndexType;
        typedef Mesh<VertexType, IndexType>         MeshType;
        typedef typename MeshType::AbstractVertex   AbstractVertex;

        MeshGeometry()
        {
        }

        MeshType & getMesh()
        {
            return mMesh;
        }

        const MeshType & getMesh() const
        {
            return mMesh;
        }

        void meshChanged()
        {
            mBoundingSphereRadius = BoundingSphereRadiusFromMesh(mMesh);
        }

    protected:
        bool doIntersectRay(const RayType &ray, IntersectionPointType &out)
        {
            GAL_imp::Solution<NumericType,2> solution2;

            if (!GAL::IntersectRaySphere(ray, mBoundingSphereRadius, solution2))
            {
                // Ray doesn't intersect bounding sphere
                return false;
            }

            const VertexType  *vertices = mMesh.getVertexPointer();
            const IndexType   *indices  = mMesh.getIndexPointer();
            size_t num = mMesh.getNumIndices();

            GAL_imp::Solution<NumericType, 3> closestSolution3;
            closestSolution3.x[0] = -1;
            const VertexType *v0;
            const VertexType *v1;
            const VertexType *v2;

            for (size_t i = 0; i < num; i += 3)
            {
                const VertexType &vA = vertices[indices[i]];
                const VertexType &vB = vertices[indices[i+1]];
                const VertexType &vC = vertices[indices[i+2]];

                const PointType &pA = AbstractVertex::getPosition(vA);
                const PointType &pB = AbstractVertex::getPosition(vB);
                const PointType &pC = AbstractVertex::getPosition(vC);

                GAL_imp::Solution<NumericType, 3> solution3;

                if (!GAL::IntersectRayTriangleByPoints(ray, pA, pB, pC, solution3))
                {
                    // No intersection at all
                    continue;
                }

                if (solution3.x[0] < 0.0001)
                {
                    // Intersection was behind ray
                    continue;
                }

                if (-1 == closestSolution3.x[0] || solution3.x[0] < closestSolution3.x[0])
                {
                    closestSolution3 = solution3;
                    v0 = &vA;
                    v1 = &vB;
                    v2 = &vC;
                }
            }

            if (-1 == closestSolution3.x[0])
            {
                return false;
            }

            NumericType u1 = closestSolution3.x[1];
            NumericType u2 = closestSolution3.x[2];

            out.normal = AbstractVertex::getNormal(*v0, *v1, *v2, u1, u2);
            out.tangent = AbstractVertex::getTangent(*v0, *v1, *v2, u1, u2);

            // distance is in unit of ray lengths
            out.distance = closestSolution3.x[0];

            out.position = ray.start + ray.direction * out.distance;

            // based on v0, v1, and v2 also texture coordinates could be calculated
            // if there was any texture

            return true;
        }

    private:
        MeshType    mMesh;
        NumericType mBoundingSphereRadius;
    };

    typedef MeshGeometry<GAL::P3f::PointType> MeshGeometry3f;
    typedef MeshGeometry<GAL::P3d::PointType> MeshGeometry3d;

#endif