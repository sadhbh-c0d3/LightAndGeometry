#ifndef INCLUDED_VERTEX_TRAITS_H
#define INCLUDED_VERTEX_TRAITS_H

#include "Linear.h"

template<class N, int I>
    struct Vertex
    {
        GAL_imp::Point<N, I> position;
        GAL_imp::Point<N, I> normal;
    };

typedef Vertex<float,3>  Vertex3f;
typedef Vertex<double,3> Vertex3d;

template<class VertexType>
    struct VertexTraits
    {
        // specialize for each vertex type
    };

template<class N, int I>
    struct VertexTraits< GAL_imp::Point<N, I> >
    {
        typedef GAL_imp::Point<N, I> VertexType;
        typedef GAL_imp::Point<N, I> PointType;

        static const PointType & getPosition(const VertexType &vertex)
        {
            return vertex;
        }
        
        static PointType getNormal(const VertexType &p0, const VertexType &p1, const VertexType &p2, N u, N v)
        {
            return GAL::Cross(p1 - p0, p2 - p0);
        }
        
        static PointType getTangent(const VertexType &p0, const VertexType &p1, const VertexType &p2, N u, N v)
        {
            return (p1 - p0);
        }
    };

template<class N, int I>
    struct VertexTraits< Vertex<N, I> >
    {
        typedef Vertex<N, I>         VertexType;
        typedef GAL_imp::Point<N, I> PointType;

        static const PointType & getPosition(const VertexType &vertex)
        {
            return vertex.position;
        }
        
        static const PointType & getNormal(const VertexType &vertex)
        {
            return vertex.normal;
        }

        static PointType getNormal(const VertexType &v0, const VertexType &v1, const VertexType &v2, N u, N v)
        {
            N s = 1 - u - v;
            
            return v0.normal * s + v1.normal * u + v2.normal * v;
        }

        static PointType getTangent(const VertexType &v0, const VertexType &v1, const VertexType &v2, N u, N v)
        {
            return (v1.position - v0.position);
        }
    };

template<class PointType>
    struct PointTraits
    {
    };

template<class N>
    struct PointTraits< GAL_imp::Point<N, 3> >
    {
        typedef GAL_imp::Point<N, 3> PointType;
        typedef N NumericType;
        enum {dim = 3};

        static const NumericType & getX(const PointType &p)
        {
            return p.x[0];
        }
        
        static const NumericType & getY(const PointType &p)
        {
            return p.x[1];
        }
        
        static const NumericType & getZ(const PointType &p)
        {
            return p.x[2];
        }
    };

template<class N>
    struct PointTraits< GAL_imp::Point<N, 4> >
    {
        typedef GAL_imp::Point<N, 4> PointType;
        typedef N NumericType;
        enum {dim = 4};

        static const NumericType & getX(const PointType &p)
        {
            return p.x[0];
        }
        
        static const NumericType & getY(const PointType &p)
        {
            return p.x[1];
        }
        
        static const NumericType & getZ(const PointType &p)
        {
            return p.x[2];
        }

        static const NumericType & getW(const PointType &p)
        {
            return p.x[3];
        }
    };

#endif