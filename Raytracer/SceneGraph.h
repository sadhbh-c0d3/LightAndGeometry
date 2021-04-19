#ifndef INCLUDED_SCENE_GRAPH_H
#define INCLUDED_SCENE_GRAPH_H

#include "Mesh.h"
#include "AABBox.h"
#include "Geometry.h"
#include "Clump.h"
#include "Frustum.h"
#include "TargetBuffer.h"

template<class _NumericType> class Light;

template<class _NumericType>
    class SceneGraph
    {
    public:
        typedef _NumericType                        NumericType;
        typedef GAL_imp::Point<NumericType,3>       PointType;
        typedef GAL_imp::Point<NumericType,4>       ColorType;
        typedef GAL_imp::Matrix<NumericType,3>      TransformType;
        typedef GAL_imp::Ray<NumericType,3>         RayType;
        typedef IntersectionPoint<NumericType,3>    IntersectionPointType;
        typedef Clump<NumericType>                  ClumpType;
        typedef std::shared_ptr<ClumpType>          ClumpPtr;
        typedef std::list<ClumpPtr>                 ListType;
        typedef Light<NumericType>                  LightType;
        typedef std::shared_ptr<LightType>          LightPtr;
        typedef std::list<LightPtr>                 ListLights;

        void addClump(const ClumpPtr &clump)
        {
            mList.push_back(clump);
        }

        void addLight(const LightPtr &light)
        {
            mLights.push_back(light);
        }

        bool intersectRay(RayType ray, IntersectionPointType &out)
        {
            return doIntersectRay(ray, out);
        }

        ColorType raytrace(RayType &ray, int recursions)
        {
            IntersectionPointType intersectionPoint;

            if (!intersectRay(ray, intersectionPoint))
            {
                return ColorType();
            }

            ray.direction /= GAL::Len(ray.direction);
            intersectionPoint.normal /= GAL::Len(intersectionPoint.normal);
            intersectionPoint.tangent /= GAL::Len(intersectionPoint.tangent);

            ColorType c1 = shade(ray, intersectionPoint);

            if (0 < recursions && intersectionPoint.isReflective)
            {
                RayType reflectedRay;
                reflectedRay.start = intersectionPoint.position;
                reflectedRay.direction = GAL::Reflect(intersectionPoint.normal, ray.direction);
                reflectedRay.start += reflectedRay.direction * 0.5;

                ColorType c2 = raytrace(reflectedRay, recursions - 1);

                c1 *= 0.7;
                c1 += c2 * 0.4;
                c1.MultiplyComponents<3>(intersectionPoint.color);
                return c1;

            }
            else {
                c1.MultiplyComponents<3>(intersectionPoint.color);
                return c1;
            }
        }

        ColorType shade(RayType &ray, IntersectionPointType &intersectionPoint)
        {
            ColorType c;
            ListLights::const_iterator it = mLights.begin();

            for (; it != mLights.end(); ++it)
            {
                c += (*it)->illuminate(*this, ray, intersectionPoint);
            }

            return c;
        }

        const ListLights &getLights()
        {
            return mLights;
        }
 
    protected:
        bool doIntersectRay(const RayType &ray, IntersectionPointType &out)
        {
            typedef ListType::const_iterator IteratorType;

            IntersectionPointType tmp;
            IteratorType closest = mList.end();
            out.distance = -1;

            for (IteratorType it = mList.begin(); it != mList.end(); ++it)
            {
                if (!(*it)->intersectRay(ray, tmp))
                {
                    continue;
                }

                if (-1 == out.distance || tmp.distance < out.distance)
                {
                    out = tmp;
                    closest = it;
                }
            }

            if (closest == mList.end())
            {
                return false;
            }

            return true;
        }


    private:
        ListType    mList;
        ListLights  mLights;
    };

typedef SceneGraph<float> SceneGraph3f;
typedef SceneGraph<double> SceneGraph3d;

#endif
