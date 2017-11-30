/*
*  Vector 3 - Class header file.
*  
*  Date:       21.03.2016
*  Creator:    Silvan Hau
*/
#ifndef _VEC3_H_
#define _VEC3_H_

#include "util.h"                      /* clamp, sqrt etc...    */

//---------------------------------------------------------------------------
//  Forward Declarations
//---------------------------------------------------------------------------
template <typename T> class Vec2;
template <typename T> class Vec4;
template <typename T> class Quaternion;

namespace math
{
    //---------------------------------------------------------------------------
    //  3D - Vector Class
    //---------------------------------------------------------------------------
    template <typename T>
    class Vec3
    {
        T data[3];

    public:
        //Constructor
        explicit Vec3(T x = 0, T y = 0, T z = 0) { data[0] = x; data[1] = y; data[2] = z; }
        explicit Vec3(const Vec2<T> & v2) { x() = v2.x(); y() = v2.y(); z() = 0; }
        explicit Vec3(const Vec4<T> & v4) { x() = v4.x(); y() = v4.y(); z() = v4.z(); }

        //Operators
        Vec3<T>     operator+           (const Vec3<T> &)          const;
        Vec3<T>     operator-           (const Vec3<T> &)          const;
        Vec3<T>     operator*           (T)                        const;
        Vec3<T>     operator/           (T)                        const;
        Vec3<T>     operator-           ()                         const;
        bool        operator==          (const Vec3<T> &)          const;
        bool        operator!=          (const Vec3<T> &)          const;
        const T&    operator[]          (unsigned int)             const;
        T&          operator[]          (unsigned int);
        void        operator/=          (const Vec3<T> &);
        void        operator*=          (const Vec3<T> &);
        void        operator/=          (const T &);
        void        operator*=          (const T &);
        void        operator+=          (const Vec3<T> &);
        void        operator-=          (const Vec3<T> &);

        //Conversion Functions
        explicit operator Vec2<T>() const { return Vec2<T>(x(), y()); }
        explicit operator Vec4<T>() const { return Vec4<T>(x(), y(), z(), 0); }

        //Const member functions
        T               magnitude       ()                          const;          //Returns the length of this vector.
        T               sqrtMagnitude   ()                          const;          //Returns the squared magnitude of this vector.
        Vec3<T>         normalized      ()                          const;          //Returns the normalized vector with a length of 1.
        T               dot             (const Vec3<T> &)           const;          //Standard dot-product.
        T               angle           (const Vec3<T> &)           const;          //Calculates the angle between both vectors. Optimized for floating-point calculation.
        T               distance        (const Vec3<T> &)           const;          //Calculates the distance between both vectors.
        Vec3<T>         maxVec          (const Vec3<T> &)           const;          //Returns a vector that is made from the largest components of both vectors.
        Vec3<T>         minVec          (const Vec3<T> &)           const;          //Returns a vector that is made from the smallest components of both vectors.
        Vec3<T>         clampMagnitude  (float)                     const;          //Returns a copy of vector with its magnitude clamped to maxLength.
        Vec3<T>         cross           (const Vec3<T> &)           const;          //Cross-product.
        Vec3<T>         project         (const Vec3<T> &)           const;          //Project the vector on the given vector argument.
        Vec3<T>         slerp           (const Vec3<T> &, float)    const;          //Spherical Interpolation - not really tested and copied from the internet
        Quaternion<T>   toQuaternion    ()                          const;          //Returns a rotation that rotates z degrees around the z axis, x degrees around the x axis, and y degrees around the y axis (in that order).
        T               maxValue        ()                          const;          //Return the maximum value of the components from this vector
        T               minValue        ()                          const;          //Return the minimum value of the components from this vector
        const T&        x               ()                          const;          //Return the First-Value of this vector.
        const T&        y               ()                          const;          //Return the Second-Value of this vector.
        const T&        z               ()                          const;          //Return the Third-Value of this vector.
        const T&        r               ()                          const;          //Return the First-Value of this vector.
        const T&        g               ()                          const;          //Return the Second-Value of this vector.
        const T&        b               ()                          const;          //Return the Third-Value of this vector.
        const T&        u               ()                          const;          //Return the First-Value of this vector.
        const T&        v               ()                          const;          //Return the Second-Value of this vector.
        const T&        w               ()                          const;          //Return the Third-Value of this vector.
        bool            isValid         ()                          const;          //Basically checks if the values are not FLT_MAX

        //Nonconst member functions
        void            normalize       ();                                         //Make this vector have a magnitude of 1.
        void            set             (T, T, T);                                  //Set x, y and z to newX, newY and newZ.
        void            scale           (T);                                        //Scale this vector by the given amount.
        void            scale           (const Vec3<T> &);                          //Scale this vector by the corresponding field of vector v.
        void            moveTowards     (const Vec3<T> &, float);                   //Moves the point towards the target point.
        T&              x               ();                                         //Return the First-Value of this vector by reference.
        T&              y               ();                                         //Return the Second-Value of this vector by reference.
        T&              z               ();                                         //Return the Third-Value of this vector by reference.
        T&              r               ();                                         //Return the First-Value of this vector by reference.
        T&              g               ();                                         //Return the Second-Value of this vector by reference.
        T&              b               ();                                         //Return the Third-Value of this vector by reference.
        T&              u               ();                                         //Return the First-Value of this vector by reference.
        T&              v               ();                                         //Return the Second-Value of this vector by reference.
        T&              w               ();                                         //Return the Third-Value of this vector by reference.

        //Static members
        static const Vec3<T> up;                                                    //same as ( 0, 1, 0)
        static const Vec3<T> down;                                                  //same as ( 0,-1, 0)
        static const Vec3<T> left;                                                  //same as ( 1, 0, 0)
        static const Vec3<T> right;                                                 //same as (-1, 0, 0)
        static const Vec3<T> forward;                                               //same as ( 0, 0, 1)
        static const Vec3<T> back;                                                  //same as ( 0, 0,-1)
        static const Vec3<T> one;                                                   //same as ( 1, 1, 1)
        static const Vec3<T> zero;                                                  //same as ( 0, 0, 0)
        static const Vec3<T> INVALID;                                               //same as (FLT_MAX,FLT_MAX,FLT_MAX)

    };

    //---------------------------------------------------------------------------
    //  Static Member Initialization
    //---------------------------------------------------------------------------
    template <typename T> const Vec3<T> Vec3<T>::up      = Vec3<T>(0.0f, 1.0f, 0.0f);
    template <typename T> const Vec3<T> Vec3<T>::down    = Vec3<T>(0.0f, -1.0f, 0.0f);
    template <typename T> const Vec3<T> Vec3<T>::left    = Vec3<T>(1.0f, 0.0f, 0.0f);
    template <typename T> const Vec3<T> Vec3<T>::right   = Vec3<T>(-1.0f, 0.0f, 0.0f);
    template <typename T> const Vec3<T> Vec3<T>::forward = Vec3<T>(0.0f, 0.0f, 1.0f);
    template <typename T> const Vec3<T> Vec3<T>::back    = Vec3<T>(0.0f, 0.0f, -1.0f);
    template <typename T> const Vec3<T> Vec3<T>::one     = Vec3<T>(1.0f, 1.0f, 1.0f);
    template <typename T> const Vec3<T> Vec3<T>::zero    = Vec3<T>(0.0f, 0.0f, 0.0f);
    template <typename T> const Vec3<T> Vec3<T>::INVALID = Vec3<T>(FLT_MAX, FLT_MAX, FLT_MAX);

    //---------------------------------------------------------------------------
    //  Operator Overloading
    //---------------------------------------------------------------------------
    template <typename T> inline
    Vec3<T> Vec3<T>::operator+(const Vec3<T> & v) const
    {
        return Vec3<T>(x() + v.x(), y() + v.y(), z() + v.z());
    }

    template <typename T> inline
    Vec3<T> Vec3<T>::operator-(const Vec3<T> & v) const
    {
        return Vec3<T>(x() - v.x(), y() - v.y(), z() - v.z());
    }

    // "*" operator for vec * scalar (member func) AND scalar * vec (nonmember func)
    template <typename T> inline
    Vec3<T> Vec3<T>::operator*(T s) const
    {
        return Vec3<T>(x() * s, y() * s, z() * s);
    }
    template <typename T, typename T2> inline
    Vec3<T> operator*(T2 s, const Vec3<T> & v)
    {
        return v * s;
    }

    template <typename T> inline
    Vec3<T> Vec3<T>::operator/(T s) const
    {
        return Vec3<T>(x() / s, y() / s, z() / s);
    }

    template <typename T> inline
    bool Vec3<T>::operator==(const Vec3<T> & v) const
    {
        return x() == v.x() && y() == v.y() && z() == v.z();
    }

    template <typename T> inline
    bool Vec3<T>::operator!=(const Vec3<T> & v) const
    {
        return !(*this == v);
    }

    template <typename T> inline
    void Vec3<T>::operator/= (const Vec3<T> & v)
    {
        x() /= v.x();
        y() /= v.y();
        z() /= v.z();
    }

    template <typename T> inline
    void Vec3<T>::operator*=(const Vec3<T> & v)
    {
        x() *= v.x();
        y() *= v.y();
        z() *= v.z();
    }

    template <typename T> inline
    void Vec3<T>::operator/= (const T & f)
    {
        x() /= f;
        y() /= f;
        z() /= f;
    }

    template <typename T> inline
    void Vec3<T>::operator*=(const T & f)
    {
        x() *= f;
        y() *= f;
        z() *= f;
    }

    template <typename T> inline
    void Vec3<T>::operator+=(const Vec3<T> & v)
    {
        x() += v.x();
        y() += v.y();
        z() += v.z();
    }

    template <typename T> inline
    void Vec3<T>::operator-=(const Vec3<T> & v)
    {
        x() -= v.x();
        y() -= v.y();
        z() -= v.z();
    }

    template <typename T> inline
    Vec3<T> Vec3<T>::operator-() const
    {
        return Vec3<T>(-x(), -y(), -z());
    }

    template <typename T> inline
    const T& Vec3<T>::operator[](unsigned int index) const
    {
        return data[index];
    }

    template <typename T> inline
    T& Vec3<T>::operator[](unsigned int index)
    {
        return data[index];
    }

    //---------------------------------------------------------------------------
    //  Const Member Functions
    //---------------------------------------------------------------------------

    //Returns the length of this vector.
    template <typename T> inline
    T Vec3<T>::magnitude() const
    {
        return sqrt(x() * x() + y() * y() + z() * z());
    }

    //Returns the squared magnitude of this vector.
    template <typename T> inline
    T Vec3<T>::sqrtMagnitude() const
    {
        return x() * x() + y() * y() + z() * z();
    }

    //Returns the normalized vector with a length of 1. Check if length = 0 necessary?
    template <typename T> inline
    Vec3<T> Vec3<T>::normalized() const
    {
        T length = this->magnitude();
        if (length == 0.0f)
            return *this;
        return Vec3<T>(x() / length, y() / length, z() / length);
    }

    //Standard dot-product between both vectors.
    template <typename T> inline
    T Vec3<T>::dot(const Vec3<T> & v) const
    {
        return x() * v.x() + y() * v.y() + z() * v.z();
    }

    //Calculates the angle between both vectors. Optimized for floating-point calculation. (Using acosf)
    template <typename T> inline
    T Vec3<T>::angle(const Vec3<T> & v) const
    {
        T res = this->magnitude() * v.magnitude();
        if (res == 0.0f)
            return 0.0f;
        return acosf(this->dot(v) / res);
    }

    //Calculates the distance between both vectors.
    template <typename T> inline
    T Vec3<T>::distance(const Vec3<T> & v) const
    {
        return (*this - v).magnitude();
    }

    //Returns a vector that is made from the largest components of two vectors.
    template <typename T> inline
    Vec3<T> Vec3<T>::maxVec(const Vec3<T> & v) const
    {
        T x, y, z;
        this->x() > v.x() ? x = this->x() : x = v.x();
        this->y() > v.y() ? y = this->y() : y = v.y();
        this->z() > v.z() ? z = this->z() : z = v.z();
        return Vec3<T>(x, y, z);
    }

    //Returns a vector that is made from the smallest components of two vectors.
    template <typename T> inline
    Vec3<T> Vec3<T>::minVec(const Vec3<T> & v) const
    {
        T x, y, z;
        this->x() < v.x() ? x = this->x() : x = v.x();
        this->y() < v.y() ? y = this->y() : y = v.y();
        this->z() < v.z() ? z = this->z() : z = v.z();
        return Vec3<T>(x, y, z);
    }

    //Returns a copy of vector with its magnitude clamped to maxLength.
    template <typename T> inline
    Vec3<T> Vec3<T>::clampMagnitude(float maxLength) const
    {
        if (this->magnitude() < maxLength)
            return *this;
        return this->normalized() * maxLength;
    }

    //Returns the cross-product (a normal vector to both vectors) between this and vector v.
    template <typename T> inline
    Vec3<T> Vec3<T>::cross(const Vec3<T> & v) const
    {
        return Vec3<T>(y() * v.z() - v.y() * z(), v.x() * z() - x() * v.z(), x() * v.y() - v.x() * y());
    }

    //Return a new vector which is projected from this vector onto the given vector argument.
    template <typename T> inline
    Vec3<T> Vec3<T>::project(const Vec3<T> & v)	const
    {
        T v_length = v.magnitude();
        if (v_length == 0.0f)
            return Vec3<T>(); //Return zero vector
        return (this->dot(v) / (v_length * v_length)) * v;
    }

    //Spherically interpolates between two vectors. Lerp amount is clamped to 0 - 1.
    template <typename T> inline
    Vec3<T> Vec3<T>::slerp(const Vec3<T> & end, float percent) const
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
        Vec3<T> relativeVec = end - *this*dot;
        relativeVec.normalize();     // Orthonormal basis
                                     // The final result.
        return ((*this*cos(theta)) + (relativeVec*sin(theta)));
    }

    //Returns a rotation that rotates z degrees around the z axis, x degrees around the x axis, and y degrees around the y axis (in that order).
    template <typename T> inline
    Quaternion<T> Vec3<T>::toQuaternion() const
    {
        T yRad = Mathf::deg2Rad(y());
        T zRad = Mathf::deg2Rad(z());
        T xRad = Mathf::deg2Rad(x());

        T c1 = cos(yRad / 2);
        T s1 = sin(yRad / 2);
        T c2 = cos(zRad / 2);
        T s2 = sin(zRad / 2);
        T c3 = cos(xRad / 2);
        T s3 = sin(xRad / 2);
        T c1c2 = c1*c2;
        T s1s2 = s1*s2;

        return Quaternion<T>(c1c2*s3 + s1s2*c3, s1*c2*c3 + c1*s2*s3, c1*s2*c3 - s1*c2*s3, c1c2*c3 - s1s2*s3).normalized();
    }

    //Return the maximum value of the components from this vector
    template <typename T> inline
    T Vec3<T>::maxValue() const
    {
        return x() > y() ? (x() > z() ? x() : z()) : (y() > z() ? y() : z());
    }

    //Return the minimum value of the components from this vector
    template <typename T> inline
    T Vec3<T>::minValue() const
    {
        return x() < y() ? (x() < z() ? x() : z()) : (y() < z() ? y() : z());
    }

    // Return the First-Value of this vector.
    template <typename T> inline
    const T& Vec3<T>::x() const
    {
        return data[0];
    }

    // Return the Second-Value of this vector.
    template <typename T> inline
    const T& Vec3<T>::y() const
    {
        return data[1];
    }

    // Return the Third-Value of this vector.
    template <typename T> inline
    const T& Vec3<T>::z() const
    {
        return data[2];
    }

    // Return the First-Value of this vector.
    template <typename T> inline
    const T& Vec3<T>::r() const
    {
        return x();
    }

    // Return the Second-Value of this vector.
    template <typename T> inline
    const T& Vec3<T>::g() const
    {
        return y();
    }

    // Return the Third-Value of this vector.
    template <typename T> inline
    const T& Vec3<T>::b() const
    {
        return z();
    }

    // Return the First-Value of this vector.
    template <typename T> inline
    const T& Vec3<T>::u() const
    {
        return x();
    }

    // Return the Second-Value of this vector.
    template <typename T> inline
    const T& Vec3<T>::v() const
    {
        return y();
    }

    // Return the Third-Value of this vector.
    template <typename T> inline
    const T& Vec3<T>::w() const
    {
        return z();
    }

    //Basically checks if the values are not FLT_MAX
    template <typename T> inline
    bool Vec3<T>::isValid() const
    {
        return x() != FLT_MAX && y() != FLT_MAX && z() != FLT_MAX;
    }

    //---------------------------------------------------------------------------
    //  Nonconst Member Functions
    //---------------------------------------------------------------------------

    //Make this vector have a magnitude of 1.
    template <typename T> inline
    void Vec3<T>::normalize()
    {
        T length = this->magnitude();
        if (length == 0.0f)
            return;
        x() /= length;
        y() /= length;
        z() /= length;
    }

    //Set x, y and z to newX, newY and newZ.
    template <typename T> inline
    void Vec3<T>::set(T newX, T newY, T newZ)
    {
        x() = newX;
        y() = newY;
        z() = newZ;
    }

    //Scale this vector by the amount of scale.
    template <typename T> inline
    void Vec3<T>::scale(T scale)
    {
        x() *= scale;
        y() *= scale;
        z() *= scale;
    }

    //Scale this vector by the corresponding field of vector v.
    template <typename T> inline
    void Vec3<T>::scale(const Vec3<T> & v)
    {
        x() *= v.x();
        y() *= v.y();
        z() *= v.z();
    }

    //Moves the point in a straight line "maxDistanceDelta" towards the target point. Can't exceed the target. Negative Values for the delta argument moves the point away (No Limit!).
    template <typename T> inline
    void Vec3<T>::moveTowards(const Vec3<T> & target, float maxDistanceDelta)
    {
        if (this->distance(target) < maxDistanceDelta)
            *this = target;
        else
            *this += ((target - *this).clampMagnitude(maxDistanceDelta));
    }


    // Return the First-Value of this vector by reference.
    template <typename T> inline
    T& Vec3<T>::x()
    {
        return data[0];
    }

    // Return the Second-Value of this vector by reference.
    template <typename T> inline
    T& Vec3<T>::y()
    {
        return data[1];
    }

    // Return the Third-Value of this vector by reference.
    template <typename T> inline
    T& Vec3<T>::z()
    {
        return data[2];
    }

    // Return the First-Value of this vector by reference.
    template <typename T> inline
    T& Vec3<T>::r()
    {
        return x();
    }

    // Return the Second-Value of this vector by reference.
    template <typename T> inline
    T& Vec3<T>::g()
    {
        return y();
    }

    // Return the Third-Value of this vector by reference.
    template <typename T> inline
    T& Vec3<T>::b()
    {
        return z();
    }

    // Return the First-Value of this vector by reference.
    template <typename T> inline
    T& Vec3<T>::u()
    {
        return x();
    }

    // Return the Second-Value of this vector by reference.
    template <typename T> inline
    T& Vec3<T>::v()
    {
        return y();
    }

    // Return the Third-Value of this vector by reference.
    template <typename T> inline
    T& Vec3<T>::w()
    {
        return z();
    }
}



#endif // _VEC3_H_

