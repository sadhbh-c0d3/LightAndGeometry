#ifndef INCLUDED_AABBOX_H
#define INCLUDED_AABBOX_H

#include "MinMax.h"
#include "Mesh.h"
#include "VertexTraits.h"

//
// Axis-aligned bounding box
//
template<class N>
    class AABBox
    {
    public:
        N xMin; N xMax;
        N yMin; N yMax;
        N zMin; N zMax;
    };


template<class MeshType>
    void AABBoxFromMesh(const MeshType &mesh, AABBox<typename MeshType::NumericType> &aaBBox)
    {
        typedef typename MeshType::AbstractVertex AbstractVertex;
        typedef typename MeshType::PointType      PointType;
        typedef typename MeshType::AbstractPoint  AbstractPoint;
        typedef typename MeshType::NumericType    NumericType;

        const VertexType *vertices = mesh.getVertexPointer();
        const size_t         count = mesh.getNumVertices();

        if (0 < count)
        {
            const VertexType &vertex = vertices[0];
            const PointType &position = AbstractVertex::getPosition(vertices[0]);

            aaBBox.xMin = AbstractPoint::getX(position);
            aaBBox.yMin = AbstractPoint::getY(position);
            aaBBox.zMin = AbstractPoint::getZ(position);
            
            aaBBox.xMax = AbstractPoint::getX(position);
            aaBBox.yMax = AbstractPoint::getY(position);
            aaBBox.zMax = AbstractPoint::getZ(position);

            if (1 < count)
            {
                for (size_t i = 1; i != count; ++i)
                {
                    const VertexType &vertex = vertices[i];
                    const PointType &position = AbstractVertex::getPosition(vertex);

                    aaBBox.xMin = Min(aaBBox.xMin, AbstractPoint::getX(position));
                    aaBBox.yMin = Min(aaBBox.yMin, AbstractPoint::getY(position));
                    aaBBox.zMin = Min(aaBBox.zMin, AbstractPoint::getZ(position));
            
                    aaBBox.xMax = Max(aaBBox.xMax, AbstractPoint::getX(position));
                    aaBBox.yMax = Max(aaBBox.yMax, AbstractPoint::getY(position));
                    aaBBox.zMax = Max(aaBBox.zMax, AbstractPoint::getZ(position));
                }
            }
        }
        else
        {
            aaBBox.xMin = aaBBox.yMin = aaBBox.zMin = 0;
            aaBBox.xMax = aaBBox.yMax = aaBBox.zMax = 0;
        }

    }


#endif