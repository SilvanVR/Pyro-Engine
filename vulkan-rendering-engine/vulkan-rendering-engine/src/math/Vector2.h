/*
*  Vector 2 - Class header file.
*
*  Date:       21.03.2016
*  Creator:    Silvan Hau
*/
#ifndef _VEC2_H_
#define _VEC2_H_

#include "util.h"  /* clamp              */

//---------------------------------------------------------------------------
//  Forward Declarations
//---------------------------------------------------------------------------
template <typename T> class Vec3;
template <typename T> class Vec4;

namespace math
{
    //---------------------------------------------------------------------------
    //  2D - Vector Class
    //---------------------------------------------------------------------------
    template <typename T>
    class Vec2
    {
        T data[2];

    public:
        //Constructor
        explicit Vec2(T x = 0, T y = 0){ data[0] = x; data[1] = y; }
        explicit Vec2 (const Vec3<T> & v3) { x() = v3.x(); y() = v3.y(); }
        explicit Vec2 (const Vec4<T> & v4) { x() = v4.x(); y() = v4.y(); }

        //Operators
        Vec2<T>       operator+     (const Vec2<T> &)           const;
        Vec2<T>       operator-     (const Vec2<T> &)           const;
        Vec2<T>       operator*     (T)                         const;
        Vec2<T>       operator/     (T)                         const;
        Vec2<T>       operator-     ()                          const;
        bool          operator==    (const Vec2<T> &)           const;
        bool          operator!=    (const Vec2<T> &)           const;
        const T&      operator[]    (unsigned int)              const;
        T&            operator[]    (unsigned int);
        void          operator+=    (const Vec2<T> &);
        void          operator-=    (const Vec2<T> &);

        //Conversion Functions
        template <typename T2>
        explicit operator Vec2<T2>() const { return Vec2<T2>(static_cast<T2>(x()), static_cast<T2>(y())); }
        explicit operator Vec3<T>() const { return Vec3<T>(x(), y(), 0); }

        //Const member functions
        T           magnitude       ()                          const;      //Returns the length of this vector.
        T           sqrtMagnitude   ()                          const;      //Returns the squared magnitude of this vector.
        Vec2<T>     normalized      ()                          const;      //Returns the normalized vector with a length of 1.
        T           dot             (const Vec2<T> &)           const;      //Standard dot-product between both vectors.
        T           angle           (const Vec2<T> &)           const;      //Calculates the angle between both vectors. Optimized for floating-point calculation.
        T           distance        (const Vec2<T> &)           const;      //Calculates the distance between both vectors.
        Vec2<T>     maxVec          (const Vec2<T> &)           const;      //Returns a vector that is made from the largest components of both vectors.
        Vec2<T>     minVec          (const Vec2<T> &)           const;      //Returns a vector that is made from the smallest components of both vectors.
        Vec2<T>     clampMagnitude  (float)                     const;      //Returns a copy of vector with its magnitude clamped to maxLength.
        Vec2<T>     slerp           (const Vec2<T> &, float)    const;      //Spherical Interpolation.
        const T&    x               ()                          const;      //Return the First-Value of this vector.
        const T&    y               ()                          const;      //Return the Second-Value of this vector.
        const T&    u               ()                          const;      //Return the First-Value of this vector.
        const T&    v               ()                          const;      //Return the Second-Value of this vector.
        bool        isValid         ()                          const;      //Basically checks if the values are not FLT_MAX

        //Nonconst member functions 
        void        normalize       ();                                     //Make this vector have a magnitude of 1.
        void        set             (T, T);                                 //Set x and y to newX and newY.
        void        scale           (T);                                    //Scale this vector by the given amount.
        void        scale           (const Vec2<T> &);                      //Scale this vector by the corresponding field of vector v.
        void        moveTowards     (const Vec2<T> &, float);               //Moves the point towards the target point.
        T&          x               ();                                     //Return the First-Value of this vector by reference.
        T&          y               ();                                     //Return the Second-Value of this vector by reference.
        T&          u               ();                                     //Return the First-Value of this vector by reference.
        T&          v               ();                                     //Return the Second-Value of this vector by reference.

        //Static members
        static const Vec2<T> up;                                            //same as ( 0, 1)
        static const Vec2<T> down;                                          //same as ( 0,-1)
        static const Vec2<T> left;                                          //same as ( 1, 0)
        static const Vec2<T> right;                                         //same as (-1, 0)
        static const Vec2<T> INVALID;                                       //same as (FLT_MAX,FLT_MAX)
    };

    //---------------------------------------------------------------------------
    //  Static Member Initialization
    //---------------------------------------------------------------------------
    template <typename T> const Vec2<T> Vec2<T>::up         = Vec2<T>( 0.0f,  1.0f);
    template <typename T> const Vec2<T> Vec2<T>::down       = Vec2<T>( 0.0f, -1.0f);
    template <typename T> const Vec2<T> Vec2<T>::left       = Vec2<T>( 1.0f,  0.0f);
    template <typename T> const Vec2<T> Vec2<T>::right      = Vec2<T>(-1.0f,  0.0f);
    template <typename T> const Vec2<T> Vec2<T>::INVALID    = Vec2<T>(FLT_MAX, FLT_MAX);

    //---------------------------------------------------------------------------
    //  Operator Overloading
    //---------------------------------------------------------------------------
    template <typename T> inline
    Vec2<T> Vec2<T>::operator+(const Vec2<T> & v) const
    {
        return Vec2<T>(x() + v.x(), y() + v.y());
    }

    template <typename T> inline
    Vec2<T> Vec2<T>::operator-(const Vec2<T> & v) const
    {
        return Vec2<T>(x() - v.x(), y() - v.y());
    }

    // "*" operator for vec * scalar (member func) AND scalar * vec (nonmember func)
    template <typename T> inline
    Vec2<T> Vec2<T>::operator*(T s) const
    {
        return Vec2<T>(x() * s, y() * s);
    }
    template <typename T, typename T2> inline
    Vec2<T> operator*(T2 s, const Vec2<T> & v)
    {
        return v * s;
    }

    template <typename T> inline
    Vec2<T> Vec2<T>::operator/(T s) const
    {
        return Vec2<T>(x() / s, y() / s);
    }

    template <typename T> inline
    bool Vec2<T>::operator==(const Vec2<T> & v) const
    {
        return x() == v.x() && y() == v.y();
    }

    template <typename T> inline
    bool Vec2<T>::operator!=(const Vec2<T> & v) const
    {
        return !(*this == v);
    }

    template <typename T> inline
    void Vec2<T>::operator+=(const Vec2<T> & v)
    {
        x() += v.x();
        y() += v.y();
    }

    template <typename T> inline
    void Vec2<T>::operator-=(const Vec2<T> & v)
    {
        x() -= v.x();
        y() -= v.y();
    }

    template <typename T> inline
    Vec2<T> Vec2<T>::operator-() const
    {
        return Vec2<T>(-x(), -y());
    }

    template <typename T> inline
    const T& Vec2<T>::operator[](unsigned int index) const
    {
        return data[index];
    }

    template <typename T> inline
    T& Vec2<T>::operator[](unsigned int index)
    {
        return data[index];
    }

    //---------------------------------------------------------------------------
    //  Const Member Functions
    //---------------------------------------------------------------------------

    //Returns the length of this vector.
    template <typename T> inline
    T Vec2<T>::magnitude() const
    {
        return sqrt(x() * x() + y() * y());
    }

    //Returns the squared magnitude of this vector.
    template <typename T> inline
    T Vec2<T>::sqrtMagnitude() const
    {
        return x() * x() + y() * y();
    }

    //Returns the normalized vector with a length of 1.
    template <typename T> inline
    Vec2<T> Vec2<T>::normalized() const
    {
        T length = this->magnitude();
        if (length == 0.0f)
            return *this;
        return Vec2<T>(x() / length, y() / length);
    }

    //Standard dot-product between both vectors.
    template <typename T> inline
    T Vec2<T>::dot(const Vec2<T> & v) const
    {
        return x() * v.x() + y() * v.y();
    }

    //Calculates the angle between both vectors. Optimized for floating-point calculation. (Using acosf)
    template <typename T> inline
    T Vec2<T>::angle(const Vec2<T> & v) const
    {
        T res = this->magnitude() * v.magnitude();
        if (res == 0.0f)
            return 0.0f;
        return acosf(this->dot(v) / res);
    }

    //Calculates the distance between both vectors.
    template <typename T> inline
    T Vec2<T>::distance(const Vec2<T> & v) const
    {
        return (*this - v).magnitude();
    }

    //Returns a vector that is made from the largest components of both vectors.
    template <typename T> inline
    Vec2<T> Vec2<T>::maxVec(const Vec2<T> & v) const
    {
        T x, y;
        this->x() > v.x() ? x = this->x() : x = v.x();
        this->y() > v.y() ? y = this->y() : y = v.y();
        return Vec2<T>(x, y);
    }

    //Returns a vector that is made from the smallest components of both vectors.
    template <typename T> inline
    Vec2<T> Vec2<T>::minVec(const Vec2<T> & v) const
    {
        T x, y;
        this->x() < v.x() ? x = this->x() : x = v.x();
        this->y() < v.y() ? y = this->y() : y = v.y();
        return Vec2<T>(x, y);
    }

    //Returns a copy of vector with its magnitude clamped to maxLength.
    template <typename T> inline
    Vec2<T> Vec2<T>::clampMagnitude(float maxLength) const
    {
        if (this->magnitude() < maxLength)
            return *this;
        return this->normalized() * maxLength;
    }

    //Spherically interpolates between two vectors. Lerp amount is clamped to 0 - 1.
    template <typename T> inline
    Vec2<T> Vec2<T>::slerp(const Vec2<T> & end, float percent) const
    {
        Mathf::clamp(percent, 0.0f, 1.0f);
        float dot = this->dot(end);
        // Clamp it to be in the range of Acos()
        // This may be unnecessary, but floating point
        // precision can be a fickle mistress.
        Mathf::clamp(dot, -1.0f, 1.0f);
        // Acos(dot) returns the angle between start and end,
        // And multiplying that by percent returns the angle between
        // start and the final result.
        float theta = acosf(dot)*percent;
        Vec2<T> relativeVec = end - *this*dot;
        relativeVec.normalize();     // Orthonormal basis
                                     // The final result.
        return ((*this*cos(theta)) + (relativeVec*sin(theta)));
    }

    // Return the First-Value of this vector.
    template <typename T> inline
    const T& Vec2<T>::x() const
    {
        return data[0];
    }

    // Return the Second-Value of this vector.
    template <typename T> inline
    const T& Vec2<T>::y() const
    {
        return data[1];
    }

    // Return the First-Value of this vector.
    template <typename T> inline
    const T& Vec2<T>::u() const
    {
        return x();
    }

    // Return the Second-Value of this vector.
    template <typename T> inline
    const T& Vec2<T>::v() const
    {
        return y();
    }

    //Basically checks if the values are not FLT_MAX
    template <typename T> inline
    bool Vec2<T>::isValid() const
    {
        return x() != FLT_MAX && y() != FLT_MAX;
    }

    //---------------------------------------------------------------------------
    //  Nonconst Member Functions
    //---------------------------------------------------------------------------

    //Make this vector have a magnitude of 1. Check if length = 0 necessary?
    template <typename T> inline
    void Vec2<T>::normalize()
    {
        T length = this->magnitude();
        if (length == 0.0f)
            return;
        x() /= length;
        y() /= length;
    }

    //Set x and y to newX and newY.
    template <typename T> inline
    void Vec2<T>::set(T newX, T newY)
    {
        x() = newX;
        y() = newY;
    }

    //Scale this vector by the amount of scale.
    template <typename T> inline
    void Vec2<T>::scale(T scale)
    {
        x() *= scale;
        y() *= scale;
    }

    //Scale this vector by the corresponding field of vector v.
    template <typename T> inline
    void Vec2<T>::scale(const Vec2<T> & v)
    {
        x() *= v.x();
        y() *= v.y();
    }

    //Moves the point in a straight line "maxDistanceDelta" towards the target point. Can't exceed the target. Negative Values for the delta argument moves the point away (No Limit!).
    template <typename T> inline
    void Vec2<T>::moveTowards(const Vec2<T> & target, float maxDistanceDelta)
    {
        if (this->distance(target) < maxDistanceDelta)
            *this = target;
        else
            *this += ((target - *this).clampMagnitude(maxDistanceDelta));
    }

    // Return the First-Value of this vector by reference.
    template <typename T> inline
    T& Vec2<T>::x()
    {
        return data[0];
    }

    // Return the Second-Value of this vector by reference.
    template <typename T> inline
    T& Vec2<T>::y()
    {
        return data[1];
    }

    // Return the First-Value of this vector by reference.
    template <typename T> inline
    T& Vec2<T>::u()
    {
        return data[0];
    }

    // Return the Second-Value of this vector by reference.
    template <typename T> inline
    T& Vec2<T>::v()
    {
        return data[1];
    }
}
#endif // _VEC2_H_