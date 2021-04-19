#ifndef INCLUDED_BOUNDING_SPHERE
#define INCLUDED_BOUNDING_SPHERE

#include "MinMax.h"
#include "Mesh.h"
#include "VertexTraits.h"

template<class MeshType>
    typename MeshType::NumericType BoundingSphereRadiusFromMesh(const MeshType &mesh)
    {
        typedef typename MeshType::VertexType     VertexType;
        typedef typename MeshType::AbstractVertex AbstractVertex;
        typedef typename MeshType::PointType      PointType;
        typedef typename MeshType::AbstractPoint  AbstractPoint;
        typedef typename MeshType::NumericType    NumericType;

        const VertexType *vertices = mesh.getVertexPointer();
        const size_t         count = mesh.getNumVertices();

        if (0 < count)
        {
            NumericType squaredRadius = 0;

            for (size_t i = 0; i != count; ++i)
            {
                const VertexType &vertex = vertices[i];
                const PointType &position = AbstractVertex::getPosition(vertex);

                NumericType sqrDistanceFromOrigin = GAL::SqrLen(position);
                squaredRadius = Max(squaredRadius, sqrDistanceFromOrigin);
            }

            NumericType radius = sqrt(squaredRadius);
            return radius;
        }
        else 
        {
            return 0;
        }
    }


#endif