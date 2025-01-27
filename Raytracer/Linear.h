#ifndef __INCLUDED_GAL_BASE_H__
#define __INCLUDED_GAL_BASE_H__
/* FILE GAL/Linear.h
 *
 * Defines Point< Numeric_T, int > and Matrix< Numeric_T, int >
 *
 * P{dim}{type}: P1i,P1f,P1d ... P4i,P4f,P4d - for points
 * M{dim}{type}: M1i,M1f,M1d ... M4i,M4f,M4d - for matrices
 *
 * operators: +=, -=, *=( constant ), /=( constant ) - were defined
 * bool operator !() were defined as !(x1 | x2 ... xn )
 *
 * Added:
 * - Cross()  - cross product (dim=3 only)
 * - Dot()    - dot product
 * - SqrLen() - lenght * lenght
 * - Len()    - lenght
 * - Cos()    - cosine of angle between two vectors
 *
 * - Matrix::T()      - matrix transposition by reference to matrix elements
 * - Matrix::Row()    - reference to matrix row
 * - Matrix::Column() - reference to matrix column
 *
 * - Point::Point( Vector )  - template ctor from any Vector (operator  [] is needed)
 * - Matrix::Matrix( Array ) - template ctor from any Array  (operator[][] is needed)
 *
 * author: Sadhbh Code (https://github.com/sadhbh-c0d3)
 * --------------------------------------------------------------*/

#include <math.h>
//------------------------------------
namespace GAL_imp { // Geometia z Algebra Liniowa (implementacja)

    template< class T >
        struct Op_Add { void operator ()( T &x, const T &y ) const { x+=y; } };

    template< class T >
        struct Op_Sub { void operator ()( T &x, const T &y ) const { x-=y; } };

    template< class T >
        struct Op_Mul { void operator ()( T &x, const T &y ) const { x*=y; } };

    template< class T >
        struct Op_Div { void operator ()( T &x, const T &y ) const { x/=y; } };

    template< class T >
        struct Op_And { void operator ()( T &x, const T &y ) const { x&=y; } };

    template< class T >
        struct Op_Or  { void operator ()( T &x, const T &y ) const { x|=y; } };

    template< class T >
        struct Op_Set { void operator ()( T &x, const T &y ) const { x=y; } };

    template< class T >
        struct Op_Neg { void operator ()( T &x, const T &y ) const { x=-y; } };

    //------------------------------------

    template< int > struct Int {};

    template< int I > struct apply_to_array;

    template<>
        struct apply_to_array<1>
        {
            template< class T1, class T2, class OP >
                static void binary( T1 &a, const T2 &b, const OP &op )
                {
                    op(a[0],b[0]);
                }
            template< class T, class C, class OP >
                static void unary( T &a, const C &b, const OP &op )
                {
                    op(a[0],b);
                }
            template< class C, class T, class OP >
                static void cumulate( C &a, const T &b, const OP &op )
                {
                    op(a,b[0]);
                }
            template< class T, class C, class OP >
                static void nullary( T &a, const C &b, const OP &op )
                {
                    op(a[0]);
                }
        };


    template< int I >
        struct apply_to_array
        {
            template< class T1, class T2, class OP >
                static void binary( T1 &a, const T2 &b, const OP &op )
                {
                    apply_to_array< I-1 >::binary( a, b, op );
                    op(a[I-1],b[I-1]);
                }

            template< class T, class C, class OP >
                static void unary( T &a, const C &b, const OP &op )
                {
                    int s = I;
                    apply_to_array< I-1 >::unary( a, b, op );
                    op(a[I-1],b);
                }
            template< class C, class T, class OP >
                static void cumulate( C &a, const T &b, const OP &op )
                {
                    apply_to_array< I-1 >::cumulate( a, b, op );
                    op(a,b[I-1]);
                }
            template< class T, class C, class OP >
                static void nullary( T &a, const OP &op )
                {
                    int s = I;
                    apply_to_array< I-1 >::nullary( a, op );
                    op(a[I-1]);
                }
        };

    //-----------------------------------

    template< class N, int I >
        struct Point
        {
            public:
                typedef N type;
                typedef N vector_type[I];
                enum { dim = I };

                Point()
                {
                    apply_to_array< dim >::unary( x, N(0), Op_Set<type>() );
                }

                template< class Vector >
                    Point( const Vector &v )
                    {
                        apply_to_array< dim >::binary( x, v, Op_Set<type>() );
                    }

                template< class Vector >
                    Point &operator += ( const Vector &v ) 
                    {
                        apply_to_array< dim >::binary( x, v, Op_Add<type>() );
                        return *this; 
                    }

                template< class Vector >
                    Point &operator -= ( const Vector &v ) 
                    {
                        apply_to_array< dim >::binary( x, v, Op_Sub<type>() );
                        return *this; 
                    }

                Point &operator *= ( type c ) 
                {
                    apply_to_array< dim >::unary( x, c, Op_Mul<type>() );
                    return *this; 
                }

                Point &operator /= ( type c ) 
                {
                    apply_to_array< dim >::unary( x, c, Op_Div<type>() );
                    return *this; 
                }

                template<int C>
                    Point &MultiplyComponents(const Point &other)
                    {
                        apply_to_array< C >::binary( x, other, Op_Mul<type>() );
                        return *this;
                    }

                bool operator !() const
                {
                    bool non_zero = false;
                    apply_to_array< dim >::cumulate( non_zero, x, Op_Or<bool>() );
                    return !non_zero; 
                }

                Point operator -() const
                {
                    Point p;
                    apply_to_array< dim >::binary( p.x, x, Op_Neg<type>() );
                    return p;
                }

                Point& Sqr()
                {
                    apply_to_array< dim >::binary( x, x, Op_Mul<type>() );
                    return *this; 
                }

                type Sum() const
                {
                    type value = 0;
                    apply_to_array< dim >::cumulate( value, x, Op_Add<type>() );
                    return value; 
                }

                operator type* () { return x; }
                operator const type* () const { return x; }
                type& operator[]( int i ) { return x[i]; }
                const type& operator[]( int i )  const { return x[i]; }

                type x[dim];

                template< class Vector >
                    Point operator + ( const Vector &v ) const { Point b=*this; b+=v; return b; }
                template< class Vector >
                    Point operator - ( const Vector &v ) const { Point b=*this; b-=v; return b; }

                Point operator * ( type c ) const { Point b=*this; b*=c; return b; }
                Point operator / ( type c ) const { Point b=*this; b/=c; return b; }

                Point &Set( int index, const N &value )
                {
                    x[ index ] = value;
                    return (*this);
                }
        };

    //-------------------------------------------------
    template< class N > struct P1_ : public Point< N, 1>
    {
        typedef Point< N, 1> PointType;
        P1_() {}
        P1_( N x1 ) { x[0]=x1; }
        P1_( const Point< N, 1> &p ): Point< N, 1>(p) {}
    };

    template< class N > struct P2_ : public Point< N, 2>
    {
        typedef Point< N, 2> PointType;
        P2_() {}
        P2_( N x1, N x2 ) { x[0]=x1; x[1]=x2; }
        P2_( const Point< N, 2> &p ): Point< N, 2>(p) {}
    };

    template< class N > struct P3_ : public Point< N, 3>
    {
        typedef Point< N, 3> PointType;
        P3_() {}
        P3_( N x1, N x2, N x3 ) { x[0]=x1; x[1]=x2; x[2]=x3; }
        P3_( const Point< N, 3> &p ): Point< N, 3>(p) {}
    };

    template< class N > struct P4_ : public Point< N, 4>
    {
        typedef Point< N, 4> PointType;
        P4_() {}
        P4_( N x1, N x2, N x3, N x4 ) { x[0]=x1; x[1]=x2; x[2]=x3; x[3]=x4; }
        P4_( const Point< N, 4> &p ): Point< N, 4>(p) {}
    };

};//namespace GAL_imp
//-------------------------------------------------

namespace GAL {

    typedef GAL_imp::P1_< unsigned char > P1ub;
    typedef GAL_imp::P1_< unsigned int > P1u;
    typedef GAL_imp::P1_< char > P1b;
    typedef GAL_imp::P1_< int > P1i;
    typedef GAL_imp::P1_< float > P1f;
    typedef GAL_imp::P1_< double > P1d;

    typedef GAL_imp::P2_< unsigned char > P2ub;
    typedef GAL_imp::P2_< unsigned int > P2u;
    typedef GAL_imp::P2_< char > P2b;
    typedef GAL_imp::P2_< int > P2i;
    typedef GAL_imp::P2_< float > P2f;
    typedef GAL_imp::P2_< double > P2d;

    typedef GAL_imp::P3_< unsigned char > P3ub;
    typedef GAL_imp::P3_< unsigned int > P3u;
    typedef GAL_imp::P3_< char > P3b;
    typedef GAL_imp::P3_< int > P3i;
    typedef GAL_imp::P3_< float > P3f;
    typedef GAL_imp::P3_< double > P3d;

    typedef GAL_imp::P4_< unsigned char > P4ub;
    typedef GAL_imp::P4_< unsigned int > P4u;
    typedef GAL_imp::P4_< char > P4b;
    typedef GAL_imp::P4_< int > P4i;
    typedef GAL_imp::P4_< float > P4f;
    typedef GAL_imp::P4_< double > P4d;

};//namespace GAL

//-------------------------------------------------

namespace GAL_imp {

    template< int I, class Op >
        struct Op_Cumulate
        {
            template< class C, class T >
                void operator ()( C &c, const T &x ) const
                {
                    apply_to_array<I>::cumulate( c, x, Op() );
                }
        };

    template< int I, class Op >
        struct Op_Array
        {
            template< class T, class C >
                void operator ()( T &x, const C &c ) const
                {
                    apply_to_array<I>::unary( x, c, Op() );
                }
        };

    template< int I, class Op >
        struct Op_BiArray
        {
            template< class T1, class T2 >
                void operator ()( T1 &x, const T2 &y ) const
                {
                    apply_to_array<I>::binary( x, y, Op() );
                }
        };

    //-------------------------------------------------
    template< class T > struct type_ref_maker
    {
        static T& make();
    };

    template< class T > struct type_maker
    {
        static T make();
    };

    template< class T >
        struct array_traits_function
        {
            template< class C > static int check( C c[] );
            static char check( ... );
        };

    template< class T >
        struct pointer_traits_function
        {
            template< class C > static int check( C *&c );
            static char check( ... );
        };

    template< class T >
        struct reference_traits_function
        {
            template< class C > static int check( C &c );
            static char check( ... );
        };

    template< class T >
        struct array_traits
        {
            enum 
            {
                is =
                    sizeof( array_traits_function<T>::check( type_ref_maker<T>::make() ) ) 
                    == sizeof(int)
            };
        };

    template< class T >
        struct pointer_traits
        {
            enum 
            {
                is = 
                    sizeof( pointer_traits_function<T>::check( type_ref_maker<T>::make() ) ) 
                    == sizeof(int),
            };
        };

    template< class T >
        struct reference_traits
        {
            enum 
            {
                is = 
                    sizeof( reference_traits_function<T>::check( type_maker<T>::make() ) ) 
                    == sizeof(int) 
            };
        };

    //------------------------------------------------
    template< int I, class E, class T=E** >
        struct Transposed
        {
            typedef E type;
            typedef Transposed<I,E,T> vector_type;
            enum { dim=I };

            Transposed( T &x, int n ): x(x), n(n) {}
            E& operator[]( int i ) const { return x[i][n]; }

            template< class Vector >
                Transposed& operator =( const Vector &v )
                {
                    apply_to_array< dim >::binary( *this, v, Op_Set<type>() );
                    return *this;
                }

            T &x;
            int n;
        };

    template< int I, class E, class T=E** >
        struct Transposition
        {
            typedef E type;
            typedef T array_type;
            typedef Transposed<I,E,T> vector_type;
            enum { dim=I };

            Transposition( T &x ): x(x){}
            Transposition( const T &x ): x( const_cast<T&>(x) ){} //--W/A done to avoid code copying

            Transposed<I,E,T> operator[]( int n ) const
            {
                return Transposed<I,E,T>(x,n);
            }

            T &x;
        };


    //-------------------------------------------------
    template< class N, int I >
        struct Matrix
        {
            //an msvc W/A
            template< int _I > struct array_size { enum { dim = _I }; };

            typedef N type;
            typedef N vector_type[ array_size<I>::dim ];
            typedef N array_type[ array_size<I>::dim ][ array_size<I>::dim ];

            enum { dim = I };

            Matrix()
            {
                apply_to_array< dim >::unary( x, 0, Op_Array<dim, Op_Set<type> >() );
            }

            template< class Array > 
                Matrix( const Array &a )
                {
                    apply_to_array< dim >::binary( x, a, Op_BiArray<dim, Op_Set<type> >() );
                }

            Matrix &operator +=( const Matrix &m )
            {
                apply_to_array< dim >::binary( x, m.x, Op_BiArray<dim, Op_Add<type> >() );
                return *this;
            }

            Matrix &operator -=( const Matrix &m )
            {
                apply_to_array< dim >::binary( x, m.x, Op_BiArray<dim, Op_Sub<type> >() );
                return *this;
            }

            Matrix &operator *=( type c )
            {
                apply_to_array< dim >::unary( x, c, Op_Array<dim, Op_Mul<type> >() );
                return *this;
            }

            Matrix &operator /=( type c )
            {
                apply_to_array< dim >::unary( x, c, Op_Array<dim, Op_Div<type> >() );
                return *this;
            }

            bool operator !() const
            {
                bool non_zero = false;
                apply_to_array< dim >::cumulate( non_zero, x, Op_Cumulate<dim, Op_Or<bool> >() );
                return !non_zero; 
            }

            //-- We assume that x[I][I] is packed to x[I*I]
            operator type* () { return x[0]; }
            operator const type* () const { return x[0]; }
            vector_type& operator[]( int i ) { return x[i]; }
            const vector_type& operator[]( int i )  const { return x[i]; }

            template< int IR >
                struct CProxy
                {
                    CProxy( const type** m ): _m(m) {}
                    template< int IC >
                        const type& operator[]( Int<IC> ) const
                        {
                            return _m[ IR ][ IC ];
                        }
                    const type** _m;
                };

            template< int IR >
                CProxy<IR> operator[]( Int<IR> ) const { return CProxy<IR>(x); }

            Point<N,I>& Row( int i ) { return reinterpret_cast<Point<N,I>&>(x[i]); }
            const Point<N,I>& Row( int i ) const { return reinterpret_cast<const Point<N,I>&>(x[i]); }

            Transposition< dim, type, array_type > T()
            {
                return Transposition< dim, type, array_type >(x);
            }

            Transposed< dim, type, array_type > Column( int i )
            {
                return T()[i];
            }

            template< class Vector >
                Matrix &SetColumn( int column, const Vector &v )
                {
                    Column( column ) = v;
                    return (*this);
                }

            template< class Vector >
                Matrix &SetRow( int row, const Vector &v )
                {
                    Row( row ) = v;
                    return (*this);
                }

            type x[I][I];
        };
    //-------------------------------------------------
    template< class N >
        struct M1_ : public Matrix< N, 1 >
    {
        M1_() {}
        M1_( N x11 ) 
        {
            x[0][0] = x11; 
        }
        M1_( const Matrix<N,1> &m ): Matrix<N,1>(m) {}
    };

    template< class N >
        struct M2_ : public Matrix< N, 2 >
    {
        M2_() {}
        M2_( N x11, N x12,
                N x21, N x22 ) 
        {
            x[0][0] = x11; x[0][1] = x12; 
            x[1][0] = x21; x[1][1] = x22; 
        }
        M2_( const Matrix<N,2> &m ): Matrix<N,2>(m) {}
    };

    template< class N >
        struct M3_ : public Matrix< N, 3 >
    {
        M3_() {}
        M3_( N x11, N x12, N x13,
                N x21, N x22, N x23,
                N x31, N x32, N x33 ) 
        {
            x[0][0] = x11; x[0][1] = x12; x[0][2] = x13; 
            x[1][0] = x21; x[1][1] = x22; x[1][2] = x23; 
            x[2][0] = x31; x[2][1] = x32; x[2][2] = x33; 
        }
        M3_( const Matrix<N,3> &m ): Matrix<N,3>(m) {}
    };

    template< class N >
        struct M4_ : public Matrix< N, 4 >
    {
        M4_() {}
        M4_( N x11, N x12, N x13, N x14,
                N x21, N x22, N x23, N x24,
                N x31, N x32, N x33, N x34,
                N x41, N x42, N x43, N x44 ) 
        {
            x[0][0] = x11; x[0][1] = x12; x[0][2] = x13; x[0][3] = x14; 
            x[1][0] = x21; x[1][1] = x22; x[1][2] = x23; x[1][3] = x24; 
            x[2][0] = x31; x[2][1] = x32; x[2][2] = x33; x[2][3] = x34; 
            x[3][0] = x41; x[3][1] = x42; x[3][2] = x43; x[3][3] = x44; 
        }
        M4_( const Matrix<N,4> &m ): Matrix<N,4>(m) {}
    };
};// namespace GAL_imp

//-------------------------------------------------
namespace GAL {

    typedef GAL_imp::M1_< int > M1i;
    typedef GAL_imp::M1_< float > M1f;
    typedef GAL_imp::M1_< double > M1d;

    typedef GAL_imp::M2_< int > M2i;
    typedef GAL_imp::M2_< float > M2f;
    typedef GAL_imp::M2_< double > M2d;

    typedef GAL_imp::M3_< int > M3i;
    typedef GAL_imp::M3_< float > M3f;
    typedef GAL_imp::M3_< double > M3d;

    typedef GAL_imp::M4_< int > M4i;
    typedef GAL_imp::M4_< float > M4f;
    typedef GAL_imp::M4_< double > M4d;

};// namespace GAL

//-------------------------------------------------
namespace GAL_imp {

    template < int N >
        struct Op_MulRow
        {
            template< class M, class V, int I>
                static void apply( const M &m, V &v, Int<I> ) 
                {
                    typedef typename M::type type;
                    typedef typename M::array_type array_t;
                    Point<type,I> z = v;
                    Op_MulRow< N-1 >::apply( m, v, Int<I>() );
                    apply_to_array<I>::binary( z.x, Transposition<I, type, M>(m)[N-1], Op_Mul<type>() );
                    v[N-1] = 0;
                    apply_to_array<I>::cumulate( v[N-1], z.x, Op_Add<type>() );
                }
        };

    template<>
        struct Op_MulRow<1>
        {
            template < class M, class V, int I>
                static void apply( const M &m, V &v, Int<I> ) 
                {
                    typedef typename M::type type;
                    typedef typename M::array_type array_t;
                    apply_to_array<I>::binary( v, Transposition<I, type, M>(m)[0], Op_Mul<type>() );
                    type z = v[0];
                    apply_to_array<I>::cumulate( v[0], v, Op_Add<type>() );
                    v[0] -= z;
                }
        };

    template< class M, class V, int I >
        struct Op_MulMatrix
        {
            void operator()( V &v, const M &m ) const
            {
                Op_MulRow<I>::apply( m, v, Int<I>() );
            }
        };

};// namespace GAL_imp

//-------------------------------------------------------------------

namespace GAL_imp {

    template< class N, int I >
        Point<N,I> operator * ( const Matrix<N,I> &m, Point<N,I> v )
        {
            Op_MulRow<I>::apply( m, v, Int<I>() );
            return v;
        }

    /*template< class N, int I, class E, class T >
      Point<N,I> operator * ( const Matrix<N,I> &m, Transposed<I,E,T> v )
      {
      Op_MulRow<I>::apply( m, v, Int<I>() );
      return v;
      } modifies matrix*/

    template< class N, int I >
        Matrix<N,I> operator * ( Matrix<N,I> m1, const Matrix<N,I> &m2 )
        {
            typedef Matrix<N,I> M;
            typedef Matrix<N,I>::vector_type V;
            apply_to_array<I>::unary( m1, m2, Op_MulMatrix<M,V,I>() );
            return m1;
        }

    template< class N, int I, class E, class T >
        Matrix<N,I> operator * ( Matrix<N,I> m1, const Transposition<I,E,T> &m2 )
        {
            typedef Transposition<I,E,T> M;
            typedef Matrix<N,I>::vector_type V;
            apply_to_array<I>::unary( m1, m2, Op_MulMatrix<M,V,I>() );
            return m1;
        }

};//namespace GAL_imp


namespace GAL {

    template< class N >
        N Cross( const GAL_imp::Point<N,2> &u, const GAL_imp::Point<N,2> &v )
        {
            return u[0]*v[1] - u[1]*v[0];
        }

    template< class N >
        GAL_imp::Point<N,3> Cross( const GAL_imp::Point<N,3> &u, const GAL_imp::Point<N,3> &v )
        {
            GAL_imp::Point<N,3> z;
            z[0] = u[1]*v[2] - u[2]*v[1];
            z[1] = u[2]*v[0] - u[0]*v[2];
            z[2] = u[0]*v[1] - u[1]*v[0];
            return z;
        }

    template< class N, int I >
        N Dot( const GAL_imp::Point<N,I> &u, GAL_imp::Point<N,I> v )
        {
            GAL_imp::apply_to_array<I>::binary( v.x, u.x, GAL_imp::Op_Mul<N>() );
            return v.Sum();
        }

    template< class N, int I >
        N SqrLen( GAL_imp::Point<N,I> u )
        {
            return u.Sqr().Sum();
        }

    template< class N, int I >
        N Len( const GAL_imp::Point<N,I> &u )
        {
            return sqrt( SqrLen(u) );
        }

    template< class N, int I >
        N SqrDistance( GAL_imp::Point<N,I> u,  GAL_imp::Point<N,I> v )
        {
            return (u - v).Sqr().Sum();
        }

    template< class N, int I >
        N Distance( GAL_imp::Point<N,I> u,  GAL_imp::Point<N,I> v )
        {
            return sqrt(SqrDistance(u,v));
        }

    template< class N, int I >
        double Cos( const GAL_imp::Point<N,I> &u, const GAL_imp::Point<N,I> &v )
        {
            return Dot( u, v )/sqrt( SqrLen(u) * SqrLen(v) );
        }

    // 
    // Both normal and direction MUST be unit length vectors
    //
    // Reflection is defined as follows:
    //
    //  I'' = I - 2*(N.I)*N;
    //
    template< class N, int I >
        GAL_imp::Point<N,I> Reflect(const GAL_imp::Point<N,I> &normal, const GAL_imp::Point<N,I> &direction)
        {
            return direction - normal * 2 * Dot(normal, direction);
        }

    // 
    // Both normal and direction MUST be unit length vectors
    //
    // Reflection is defined as follows:
    //
    //  I'' = I - 2*(N.I/N.N)*N;
    //
    // NOTE recipSqrNormal = 1/N.N
    //
    template< class N, int I >
        GAL_imp::Point<N,I> Reflect(
            const GAL_imp::Point<N,I> &normal,
            const GAL_imp::Point<N,I> &direction,
            N                          recipSqrNormal)
        {
            N k = 2 * GAL::Dot(normal, direction) * recipSqrNormal;
            return direction - (normal * k);
        }

    //
    // Project point onto plane
    //
    // I'' = I - (N.I/N.N)*N
    //
    // NOTE recipSqrNormal = 1/N.N
    //
    template<class N, int I>
        GAL_imp::Point<N,I> ProjectToPlane(const GAL_imp::Point<N,I> &normal, const GAL_imp::Point<N,I> &point, N recipSqrNormal)
        {
            N k = GAL::Dot(point,normal) * recipSqrNormal;
            return point - (normal * k);
        }

    template<class N, int I>
        GAL_imp::Point<N,I> ProjectToPlane(const GAL_imp::Point<N,I> &normal, const GAL_imp::Point<N,I> &point)
        {
            N recipSqrNormal = 1/GAL::Dot(normal, normal);
            N k = GAL::Dot(point,normal) * recipSqrNormal;
            return point - (normal * k);
        }
    //
    // Project point onto vector
    //
    // I'' = (N.I/N.N)*N
    //
    // NOTE recipSqrNormal = 1/N.N
    //
    template<class N, int I>
        GAL_imp::Point<N,I> Project(const GAL_imp::Point<N,I> &normal, const GAL_imp::Point<N,I> &point, N recipSqrNormal)
        {
            N k = GAL::Dot(point,normal) * recipSqrNormal;
            return normal * k;
        }

    template<class N>
        GAL_imp::Point<N,3> Orthogonal(const GAL_imp::Point<N,3> &vec)
        {
            if (vec[2] < vec[1])
            {
                return GAL_imp::P3_<N>(vec[2] * -1, 0, vec[0]);
            }
            else
            {
                return GAL_imp::P3_<N>(0, vec[2], vec[1] * -1);
            }
        }

    template<class N>
        GAL_imp::M3_<N> EulerRotationX(N angleDegrees)
        {
            N radians = angleDegrees * 3.1415 / 180.0;

            GAL_imp::M3_<N> matrix(
                1,           0,          0,
                0,  cos(radians),-sin(radians),
                0,  sin(radians), cos(radians));

            return matrix;
        }

    template< class N>
        GAL_imp::M3_<N> EulerRotationY(N angleDegrees)
        {
            N radians = angleDegrees * 3.1415 / 180.0;

            GAL_imp::M3_<N> matrix(
                cos(radians),0, -sin(radians),
                0,           1,          0,
                sin(radians),0, cos(radians));

            return matrix;
        }

    template< class N>
        GAL_imp::M3_<N> EulerRotationZ(N angleDegrees)
        {
            N radians = angleDegrees * 3.1415 / 180.0;

            GAL_imp::M3_<N> matrix(
                cos(radians),-sin(radians),0
                sin(radians),cos(radians) ,0
                0,           0,            1);

            return matrix;
        }

};//namespace GAL
#endif//__INCLUDED_GAL_BASE_H__

