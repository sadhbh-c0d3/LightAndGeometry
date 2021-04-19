#ifndef INCLUDED_LIGHT_H
#define INCLUDED_LIGHT_H

#include "Linear.h"
#include "Intersect.h"


template<class _NumericType> class SceneGraph;

template<class _NumericType>
    class Light
    {
    public:
        typedef _NumericType                        NumericType;
        typedef GAL_imp::Point<NumericType,3>       PointType;
        typedef GAL_imp::Point<NumericType,3>       LightColorType;
        typedef GAL_imp::Point<NumericType,4>       ColorType;
        typedef GAL_imp::Matrix<NumericType,3>      TransformType;
        typedef GAL_imp::Ray<NumericType,3>         RayType;
        typedef IntersectionPoint<NumericType,3>    IntersectionPointType;
        typedef SceneGraph<NumericType>             SceneGraphType;


        Light(): mShadow(false), mSoftShadowWidth(0)
        {
            NumericType coeff[3][3] =
            {
                {0.10, 0.15, 0.10},
                {0.15, 0.20, 0.15},
                {0.10, 0.15, 0.10},
            };

            for (int i=0; i<3; ++i)
            {
                for (int j=0; j<3; ++j)
                {
                    mSoftShadowCoeff[i][j] = coeff[i][j];
                }
            }
        }

        void setPosition(const PointType &t)
        {
            mPosition = t;
        }

        void setDiffuseColor(const LightColorType &c)
        {
            mDiffuseColor = c;
        }

        void setSpecularColor(const LightColorType &c)
        {
            mSpecularColor = c;
        }

        void setShadow(bool val)
        {
            mShadow = val;
        }

        bool isShadow()
        {
            return mShadow;
        }

        void setSoftShadowWidth(NumericType val)
        {
            mSoftShadowWidth = val;
        }

        NumericType getSoftShadowWidth()
        {
            return mSoftShadowWidth;
        }

        ColorType illuminate(SceneGraphType &sceneGraph, RayType &ray, IntersectionPointType &intersectionPoint)
        {
            NumericType intensity = 1.0;

            if (mShadow)
            {
                if (0 == mSoftShadowWidth)
                {
                    if (dropShadow(sceneGraph, mPosition, intersectionPoint))
                    {
                        return ColorType();
                    }
                }
                else
                {
                    intensity = 1.0 - softShadow(sceneGraph, intersectionPoint);
                }
            }

            return processPointLight(
                    mPosition,
                    intersectionPoint.position,
                    intersectionPoint.normal,
                    ray.direction,
                    mDiffuseColor * intensity,
                    mSpecularColor * intensity,
                    2, 0.8, 4, 16);
        }

        bool dropShadow(SceneGraphType &sceneGraph, const GAL::P3d &lightPosition, const IntersectionPointType &intersectionPoint)
        {
            RayType lightRay;
            lightRay.start = lightPosition;
            lightRay.direction = (intersectionPoint.position - lightPosition);

            IntersectionPointType lightIntersectionPoint;

            if (!sceneGraph.intersectRay(lightRay, lightIntersectionPoint)) {
                return false;
            }

            PointType distVect = (lightIntersectionPoint.position - intersectionPoint.position);

            return (0.00001 < distVect.Sqr().Sum());
        }

        double softShadow(SceneGraphType &sceneGraph, const IntersectionPointType &intersectionPoint)
        {
            NumericType shadowCoverage = 0.0;
            NumericType lightSmoothWitdh = 0.05;

            PointType lightZ = intersectionPoint.position - mPosition;
            lightZ /= GAL::Len(lightZ);

            PointType lightX = GAL::Orthogonal(lightZ);
            PointType lightY = GAL::Cross(lightZ, lightX);

            GAL::P3d tmpLightPosition;

            for (int iy = 0; iy < 3; ++iy)
            {
                for (int ix = 0; ix < 3; ++ix)
                {
                    double a = mSoftShadowWidth * double(ix - 1);
                    double b = mSoftShadowWidth * double(iy - 1);
                    
                    tmpLightPosition = mPosition + lightX * a + lightY * b;

                    if (dropShadow(sceneGraph, tmpLightPosition, intersectionPoint))
                    {
                        shadowCoverage += mSoftShadowCoeff[ix][iy];
                    }
                }
            }

            return shadowCoverage;
        }

    private:
        PointType       mPosition;
        LightColorType  mDiffuseColor;
        LightColorType  mSpecularColor;
        bool            mShadow;
        NumericType     mSoftShadowWidth;
        NumericType     mSoftShadowCoeff[3][3];

        //
        // FIXME: Change to use NumericType
        //
        static GAL::P4d processPointLight(
				 const PointType &lightPosition,
				 const PointType &intersectoinPoint,
				 const PointType &intersectionNormal,
				 const PointType &rayDirNormalized,
				 const PointType &diffuseColor,
				 const PointType &specularColor,
				 NumericType diffuseWidth,
				 NumericType diffuseSharpness,
				 NumericType specularWidth,
				 NumericType specularSharpness)
        {

            // Light direction as intersection point
            PointType lightDir = lightPosition - intersectoinPoint;

            // Swuare distace between light and point where ray intersected surface
            NumericType sqrLightDistance = GAL::Dot(lightDir, lightDir);

            // Normalize lightDir and rayDir
            PointType lightDirNormal = lightDir / sqrt(sqrLightDistance);

            NumericType diffuseCos = GAL::Dot(lightDirNormal, intersectionNormal);
            NumericType diffuseAngle = acos(diffuseCos);
            NumericType diffuseLight = 1 / (pow(diffuseWidth*diffuseAngle, diffuseSharpness) + 1);

            PointType reflectedRayDir = GAL::Reflect(intersectionNormal, rayDirNormalized);

            NumericType specularCos = GAL::Dot(lightDirNormal, reflectedRayDir);
            NumericType specularAngle = acos(specularCos);
            NumericType specularLight = 1 / (pow(specularWidth*specularAngle, specularSharpness) + 1);

            return GAL_imp::P4_<NumericType>(
                    (diffuseColor.x[0] * diffuseLight) + (specularColor.x[0] * specularLight),
                    (diffuseColor.x[1] * diffuseLight) + (specularColor.x[1] * specularLight),
                    (diffuseColor.x[2] * diffuseLight) + (specularColor.x[2] * specularLight),
                    1);
        }
    };



typedef Light<float>  Light3f;
typedef Light<double> Light3d;

#endif