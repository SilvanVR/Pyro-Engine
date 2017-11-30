/*
 *  Math - Helper functions.
 *  Static member declarations in corresponding .cpp file
 *
 *  Date:    23.03.2016
 *  Creator: Silvan Hau
 *  Note to me: Do performance check with different data types
 */

#ifndef _MATH_HELP_H_
#define _MATH_HELP_H_

#include <math.h>   /* sin, cos etc. */
#include <cfloat>   /* FLT_MAX */

namespace math
{

    class Mathf
    {
    public:
        static const double PI;
        static const float PI_F;
        static const float Eps;
        
        template <typename T, typename T2>  static  T&    clamp         (T &, T2, T2);                  //Clamp the given value to min and max.	
        template <typename T>               static  float deg2Rad       (const T &);                    //Converts degree to radian. Same as val * (PI * 2) / 360.
        template <typename T>               static  float rad2Deg       (const T &);                    //Converts radions to degree. Same as val * 360 / (PI * 2).
        template <typename T>               static  T     lerp          (const T &, const T &, float);  //Lerp the given values by the float amount. Lerp amount is clamped to 0 - 1.
        template <typename T>               static  T     lerpUnclamped (const T &, const T &, float);  //Lerp the given values by the float amount.
        template <typename T>               static  T     cut           (const T &);                    //Remove rest of a wrong floating-point calculation (e.g. cos(90°))
    };
    
    //---------------------------------------------------------------------------
    //  Member Functions Implementation
    //---------------------------------------------------------------------------

    //Clamp the given value to min and max and returns it for concatenation.
    template <typename T, typename T2> inline
    T& Mathf::clamp(T & value, T2 min, T2 max)
    {
        return value < min ? value = static_cast<T>(min) : (value > max ? value = static_cast<T>(max) : value);
    }

    //Converts degree to radian. Same as val * (PI / 180).
    template <typename T> inline
    float Mathf::deg2Rad(const T & val)
    {
        return val * 0.01745329251f;
    }

    //Converts radions to degree. Same as val * 360 / (PI * 2).
    template <typename T> inline
    float Mathf::rad2Deg(const T & val)
    {
        return val * 57.2957795131f;
    }

    //Lerp the 2 given values by the amount of t. T will be clamped to 0 - 1. Note: works for vectors due to operator overloading aswell.
    template <typename T> inline
    T Mathf::lerp(const T & a, const T & b, float t)
    {
        clamp(t, 0, 1);
        return a * (1 - t) + b * t;
    }

    //Lerp the 2 given values by the amount of t. T will not be clamped. Note: works for vectors due to operator overloading aswell.
    template <typename T> inline
    T Mathf::lerpUnclamped(const T & a, const T & b, float t)
    {
        return a * (1 - t) + b * t;
    }

    //Remove rest of a wrong floating-point calculation
    template <typename T> inline
    T Mathf::cut(const T & s)
    {
        if (fabs(s) < Eps)
            return 0;
        else
            return s;
    }
}

#endif