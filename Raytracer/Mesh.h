#ifndef INCLUDED_MESH_H
#define INCLUDED_MESH_H

#include <vector>
#include "VertexTraits.h"

//
// Mesh made of vertices and indices
//
template<class _VertexType, class _IndexType = int>
    class Mesh
    {
    public:
        typedef _VertexType                         VertexType;
        typedef _IndexType                          IndexType;
        typedef VertexTraits<VertexType>            AbstractVertex;
        typedef typename AbstractVertex::PointType  PointType;
        typedef PointTraits<PointType>              AbstractPoint;
        typedef typename AbstractPoint::NumericType NumericType;

        Mesh()
        {
        }

        void addVertex(const VertexType &v)
        {
            mVertices.push_back(v);
        }

        void addIndex(const IndexType &i)
        {
            mIndices.push_back(i);
        }

        const VertexType * getVertexPointer() const
        {
            return &mVertices[0];
        }

        const IndexType * getIndexPointer() const
        {
            return &mIndices[0];
        }

        size_t getNumVertices() const
        {
            return mVertices.size();
        }

        size_t getNumIndices() const
        {
            return mIndices.size();
        }

        void addVertices(VertexType *pointer, size_t number)
        {
            mVertices.reserve(mVertices.size() + number);

            for (size_t i = 0; i != number; ++i)
            {
                addVertex(pointer[i]);
            }
        }

        void addIndices(IndexType *pointer, size_t number)
        {
            mIndices.reserve(mIndices.size() + number);

            for (size_t i = 0; i != number; ++i)
            {
                addIndex(pointer[i]);
            }
        }

    private:
        std::vector<VertexType>  mVertices;
        std::vector<IndexType>   mIndices;
    };

    typedef Mesh<GAL::P3f::PointType, short> Mesh3f;
    typedef Mesh<GAL::P3d::PointType, int>   Mesh3d;

#endif