#ifndef INCLUDED_INTERSECT_H
#define INCLUDED_INTERSECT_H

#include "Linear.h"

namespace GAL_imp {

    template<class N, int I>
        struct Ray
        {
            Point<N,I> start;
            Point<N,I> direction;
        };

    template<class N, int I>
        struct Plane
        {
            Point<N,I> normal;
            N          distance;
        };

    template<class N, int I>
        struct Solution
        {
            N x[I];
        };

};

namespace GAL {
    typedef GAL_imp::Ray<float,3>       Ray3f;
    typedef GAL_imp::Ray<double,3>      Ray3d;
    typedef GAL_imp::Solution<float,2>  Solution2f;
    typedef GAL_imp::Solution<double,2> Solution2d;
    typedef GAL_imp::Solution<float,3>  Solution3f;
    typedef GAL_imp::Solution<double,3> Solution3d;

    template<class N>
        N ChooseNearestPositiveRoot(const GAL_imp::Solution<N,2> &solution)
        {   
            //
            // Possible cases are:
            // - solution is behind ray
            // - ray begins inside of sphere
            // - sphere is in front of ray
            //
            // Returns negative number if sphere is behind ray.
            //
            return (0 < solution.x[0] ? solution.x[0] : solution.x[1]);
        }

    //
    // Solves quadratic equation of form: A*x*x + B*x + C = 0
    //
    // Quadratic equation may have two roots:
    //	    x = (-B - sqrt( B - 4 A C )) / 2 A
    //      x = (-B + sqrt( B - 4 A C )) / 2 A
    //
    // Since this function is used to test intersection between ray and sphere
    // when ray is tangent with sphere no intersection is assumed. So there are
    // always two roots or none.
    //
    // Solution roots are sorted in ascending order
    //
    template<class N>
        bool SolveQuadratic(N A, N B, N C, GAL_imp::Solution<N,2> &solution)
        {
            double _4AC = 4 * A * C;
            double _B2  = B * B;

            // Solution does not exist, or exactly one solution exists.
            if (_B2 <= _4AC)
            {
                return false;
            }

            N sqrtDelta = sqrt(_B2 - _4AC);
            N recip2A = 1 / (2*A);

            // Always: 0 < sqrtDelta, hence always: x[0] < x[1]
            solution.x[0] = (-sqrtDelta - B) * recip2A;
            solution.x[1] = ( sqrtDelta - B) * recip2A;

            return true;
        }

    template<class N, int I>
        bool IntersectRayPlane(
            const GAL_imp::Ray<N,I>     &ray,
            const GAL_imp::Point<N,I>   &normal,
            N                           &solution)
        {
            N recipSqrNormal = 1/GAL::Dot(normal, normal);

            N von = GAL::Dot(normal, ray.direction);
            if (0 == von)
            {  
                // Ray is parallel
                return false;
            }

            N uon = GAL::Dot(normal, ray.start);

            solution = - uon / von;

            return true;
        }

    //
    // Intersect ray defined by: ray.start, ray.dir with sphere with center at (0,0,0) and radius sphereRadius.
    //
    // Returns true if there are intersections, and: ray.start + ray.dir * t is the intersection point for t = t1 and t2.
    //
    template<class N, int I>
        bool IntersectRaySphere(const GAL_imp::Ray<N,I> &ray, N sphereRadius, GAL_imp::Solution<N,2> &solution)
        {
            //
            // Solve quadratic equation of form:
            //
            //	t^2 v.v + 2 u.v t + u.u - r^2 = 0
            //
            // where:
            //	t is variable we're looking for
            //	v is ray direction
            //	u is ray start relative to sphere center
            //	r is sphere radius
            //
            // We are going to solve quadratic equation: a*x*x + b*x + c = 0 where:
            //	a = v.v
            //	b = 2 u.v
            //	c = u.u - r^2
            //

            N a = Dot(ray.direction, ray.direction);
            N b = 2 * Dot(ray.start, ray.direction);
            N c = Dot(ray.start, ray.start) - (sphereRadius * sphereRadius);

            return SolveQuadratic(a, b, c, solution);
        }

    template<class N, int I>
        bool IntersectRayInfiniteCylinder(
            const GAL_imp::Ray<N,I>    &ray,
            N                           cylinderRadius,
            const GAL_imp::Point<N,I>  &cylinderAxis,
            GAL_imp::Solution<N,2>     &solution)
        {
            //
            // Reduce problem of intersecting I dimensional ray with
            // infinite cylinder to intersecting I-1 dimensional
            // ray with I-1 dimensional sphere.
            //
            // Simply: Project ray onto plane orthogonal to cylinder axis.
            // Projected cylinder will be I-1 dimensional sphere.
            //
            // NOTE For I=3 we'll get circle, however this information is meaningless.
            //

            N sqrAxis = GAL::Dot(cylinderAxis, cylinderAxis);
            if (0 == sqrAxis)
            {
                // 0-length axis
                return false;
            }

            N von = GAL::Dot(cylinderAxis, ray.direction);
            if (0 == von)
            {
                // ray is parallel to axis
                return false;
            }
            N uon = GAL::Dot(cylinderAxis, ray.start);
            N recipSqrAxis = 1 / sqrAxis;

            GAL_imp::Ray<N,I> planarRay;
            planarRay.start     = ray.start     - cylinderAxis * (uon * recipSqrAxis);
            planarRay.direction = ray.direction - cylinderAxis * (von * recipSqrAxis);

            return IntersectRaySphere(planarRay, cylinderRadius, solution);
        }


    //
    // This is fast ray-triangle intersection algorithm.
    //
    // Ray is:
    //      p = u + v*t
    //
    // Triangle is:
    //      p0, e1, e2
    //
    // Solution is:
    //      t, u, v
    //
    template<class N>
        bool IntersectRayTriangleByEdges(
                const GAL_imp::Ray<N,3>   &ray,
                const GAL_imp::Point<N,3> &triPos,
                const GAL_imp::Point<N,3> &triEdge1,
                const GAL_imp::Point<N,3> &triEdge2,
                GAL_imp::Solution<N,3>    &solution)
        {
            GAL_imp::Point<N,3> _P = Cross(ray.direction, triEdge2);
            N d1 = Dot(_P, triEdge1);

            if (d1 < 0.00001) {
                // if determinant is near zero ray lies in plane of triangle
                return false;
            }

            // calculate distance from vert to ray origin
            GAL_imp::Point<N,3> _T = ray.start - triPos;

            // calculate U parameter and test bounds
            N d3 = Dot(_P, _T);
            if (d3 < 0 || d3 > d1) {
                return false;
            }

            GAL_imp::Point<N,3> _Q = Cross(_T, triEdge1);

            //calculate V parameter and test bounds
            N d4 = Dot(_Q, ray.direction);
            if (d4 < 0 || d3 + d4 > d1) {
                return false;
            }

            N d2 = Dot(_Q, triEdge2);
            N f = 1 / d1;

            solution.x[0] = f * d2;
            solution.x[1] = f * d3;
            solution.x[2] = f * d4;

            return true;
        }

    template<class N>
        bool IntersectRayTriangleByPoints(
                const GAL_imp::Ray<N,3>   &ray,
                const GAL_imp::Point<N,3> &A,
                const GAL_imp::Point<N,3> &B,
                const GAL_imp::Point<N,3> &C,
                GAL_imp::Solution<N,3>    &solution)
        {
            return IntersectRayTriangleByEdges(ray, A, B - A, C - A, solution);
        }

    template<class N>
        void PlaneFromEdges(
            const GAL_imp::Point<N, 3> &P,
            const GAL_imp::Point<N, 3> &e1,
            const GAL_imp::Point<N, 3> &e2,
            GAL_imp::Plane<N, 3> &plane)
        {
            plane.normal = Cross(e1, e2);
            plane.distance = -Dot(plane.normal, P);
        }

    template<class N>
        void PlaneFromPoints(
            const GAL_imp::Point<N, 3> &A,
            const GAL_imp::Point<N, 3> &B,
            const GAL_imp::Point<N, 3> &C,
            GAL_imp::Plane<N, 3> &plane)
        {
            PlaneFromEdges(A, B - A, C - A, plane);
        }

    template<class N, int I>
        void NormalizePlane(GAL_imp::Plane<N, I> &plane)
        {
            N magnitude = sqrt(Dot(plane.normal, plane.normal));
            plane.normal /= magnitude;
            plane.distance /= magnitude;
        }

    //
    // Calculate signed distance between point and plane.
    // If plane is not normalized distance is not correct,
    // however the sign will be correct.
    //
    template<class N, int I>
        N SignedDistancePlanePoint(
            const GAL_imp::Plane<N, I> &plane,
            const GAL_imp::Point<N, I> &point)
        {
            return Dot(plane.normal, point) + plane.distance;
        }

};

#endif
