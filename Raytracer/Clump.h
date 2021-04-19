#ifndef INCLUDED_CLUMP_H
#define INCLUDED_CLUMP_H

#include <memory>
#include <list>

#include "Geometry.h"

template<class _NumericType>
    class Clump
    {
    public:
        typedef _NumericType                        NumericType;
        typedef GAL_imp::Point<NumericType,3>       PointType;
        typedef GAL_imp::Matrix<NumericType,3>      TransformType;
        typedef GAL_imp::Ray<NumericType,3>         RayType;
        typedef IntersectionPoint<NumericType,3>    IntersectionPointType;
        typedef Geometry<NumericType>               GeomertryType;
        typedef std::shared_ptr<GeomertryType>      GeomertryPtr;
        typedef std::list<GeomertryPtr>             ListType;

        void addGeometry(const GeomertryPtr &geometry)
        {
            mList.push_back(geometry);
        }

        bool intersectRay(RayType ray, IntersectionPointType &out)
        {
            // TODO: Clump could have bounding sphere and LTM

            return doIntersectRay(ray, out);
        }

    protected:
        bool doIntersectRay(const RayType &ray, IntersectionPointType &out)
        {
            typedef typename ListType::const_iterator IteratorType;
            
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
        ListType mList;
    };
    
    typedef Clump<float>  Clump3f;
    typedef Clump<double> Clump3d;

#endif