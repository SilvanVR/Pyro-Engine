/*
*  Vector 4 - Class header file.
*  
*  Date:      21.03.2016
*  Creator:   Silvan Hau
*/
#ifndef _VEC4_H_
#define _VEC4_H_

#include "Vector2.h"                /* Convert to vec2    */
#include "Vector3.h"                /* Convert to vec2    */
#include "util.h"                   /* clamp, sqrt etc.   */

//---------------------------------------------------------------------------
//  Forward Declarations
//---------------------------------------------------------------------------
template <typename T> class Vec2;
template <typename T> class Vec3;

namespace math
{
    //---------------------------------------------------------------------------
    //  4D - Vector Class
    //---------------------------------------------------------------------------
    template <typename T>
    class Vec4
    {
        T data[4];

    public:
        //Constructors
        explicit Vec4(T x = 0, T y = 0, T z = 0, T w = 0) { data[0] = x; data[1] = y; data[2] = z; data[3] = w; }
        explicit Vec4(const Vec2<T> & v2)       { x() = v2.x(); y() = v2.y(); z() = 0; w() = 0; }
        explicit Vec4(const Vec3<T> & v3)       { x() = v3.x(); y() = v3.y(); z() = v3.z(); w() = 0; }
        explicit Vec4(const Vec3<T> & v3, T s)  { x() = v3.x(); y() = v3.y(); z() = v3.z(); w() = s; }

        //Operators
        Vec4<T>     operator+       (const Vec4<T> &)                   const;
        Vec4<T>     operator-       (const Vec4<T> &)                   const;
        Vec4<T>     operator*       (T)                                 const;
        Vec4<T>     operator/       (T)                                 const;
        Vec4<T>     operator-       ()                                  const;
        bool        operator==      (const Vec4<T> &)                   const;
        bool        operator!=      (const Vec4<T> &)                   const;
        const T&    operator[]      (unsigned int)                      const;
        T&          operator[]      (unsigned int);
        void        operator/=      (const Vec4<T> &);
        void        operator*=      (const Vec4<T> &);
        void        operator/=      (const T &);
        void        operator*=      (const T &);
        void        operator+=      (const Vec4<T> &);
        void        operator-=      (const Vec4<T> &);

        //Conversion Functions
        explicit operator Vec3<T>() const { return Vec3<T>(x(), y(), z()); }

        //Const member functions
        T           magnitude       ()                                  const;       //Returns the length of this vector.
        T           sqrtMagnitude   ()                                  const;       //Returns the squared magnitude of this vector.
        Vec4<T>     normalized      ()                                  const;       //Returns the normalized vector with a length of 1.
        T           dot             (const Vec4<T> &)                   const;       //Standard dot-product.
        T           angle           (const Vec4<T> &)                   const;       //Calculates the angle between both vectors. Optimized for floating-point calculation.
        T           distance        (const Vec4<T> &)                   const;       //Calculates the distance between both vectors.
        Vec4<T>     maxVec          (const Vec4<T> &)                   const;       //Returns a vector that is made from the largest components of both vectors.
        Vec4<T>     minVec          (const Vec4<T> &)                   const;       //Returns a vector that is made from the smallest components of both vectors.
        Vec4<T>     clampMagnitude  (float)                             const;       //Returns a copy of vector with its magnitude clamped to maxLength.
        Vec4<T>     project         (const Vec4<T> &)                   const;       //Project the vector on the given vector argument.
        Vec4<T>     slerp           (const Vec4<T> &, float)            const;       //Spherical Interpolation - not really tested and copied from the internet
        const T&    x               ()                                  const;       //Return the First-Value of this vector.
        const T&    y               ()                                  const;       //Return the Second-Value of this vector.
        const T&    z               ()                                  const;       //Return the Third-Value of this vector.
        const T&    w               ()                                  const;       //Return the Fourth-Value of this vector.
        const T&    r               ()                                  const;       //Return the First-Value of this vector.
        const T&    g               ()                                  const;       //Return the Second-Value of this vector.
        const T&    b               ()                                  const;       //Return the Third-Value of this vector.
        const T&    a               ()                                  const;       //Return the Fourth-Value of this vector.
        bool        isValid         ()                                  const;       //Basically checks if the values are not FLT_MAX

        //Nonconst member functions
        void        normalize       ();                                              //Make this vector have a magnitude of 1.
        void        set             (T, T, T, T);                                    //Set x, y, z and w to newX, newY, newZ and newW.
        void        scale           (T);                                             //Scale this vector by the given amount.
        void        scale           (const Vec4<T> &);                               //Scale this vector by the corresponding field of vector v.
        void        moveTowards     (const Vec4<T> &, float);                        //Moves the point towards the target point.
        T&          x               ();                                              //Return the First-Value of this vector by reference.
        T&          y               ();                                              //Return the Second-Value of this vector by reference.
        T&          z               ();                                              //Return the Third-Value of this vector by reference.
        T&          w               ();                                              //Return the Fourth-Value of this vector by reference.
        T&          r               ();                                              //Return the First-Value of this vector by reference.
        T&          g               ();                                              //Return the Second-Value of this vector by reference.
        T&          b               ();                                              //Return the Third-Value of this vector by reference.
        T&          a               ();                                              //Return the Fourth-Value of this vector by reference.

    };

    //---------------------------------------------------------------------------
    //  Operator Overloading
    //---------------------------------------------------------------------------
    template <typename T> inline
    Vec4<T> Vec4<T>::operator+(const Vec4<T> & v) const
    {
        return Vec4<T>(x() + v.x(), y() + v.y(), z() + v.z(), w() + v.w());
    }

    template <typename T> inline
    Vec4<T> Vec4<T>::operator-(const Vec4<T> & v) const
    {
        return Vec4<T>(x() - v.x(), y() - v.y(), z() - v.z(), w() - v.w());
    }

    // "*" operator for vec * scalar (member func) AND scalar * vec (nonmember func)
    template <typename T> inline
    Vec4<T> Vec4<T>::operator*(T s) const
    {
        return Vec4<T>(x() * s, y() * s, z() * s, w() * s);
    }
    template <typename T, typename T2> inline
    Vec4<T> operator*(T2 s, const Vec4<T> & v)
    {
        return v * s;
    }
    // "/" operator for vec / scalar (member func)
    template <typename T> inline
    Vec4<T> Vec4<T>::operator/(T s) const
    {
        return Vec4<T>(x() / s, y() / s, z() / s, w() / s);
    }

    template <typename T> inline
    bool Vec4<T>::operator==(const Vec4<T> & v) const
    {
        return x() == v.x() && y() == v.y() && z() == v.z() && w() == v.w();
    }

    template <typename T> inline
    bool Vec4<T>::operator!=(const Vec4<T> & v) const
    {
        return !(*this == v);
    }

    template <typename T> inline
    void Vec4<T>::operator/= (const Vec4<T> & v)
    {
        x() /= v.x();
        y() /= v.y();
        z() /= v.z();
        w() /= v.w();
    }

    template <typename T> inline
    void Vec4<T>::operator*=(const Vec4<T> & v)
    {
        x() *= v.x();
        y() *= v.y();
        z() *= v.z();
        w() *= v.w();
    }

    template <typename T> inline
    void Vec4<T>::operator/= (const T & f)
    {
        x() /= f;
        y() /= f;
        z() /= f;
        w() /= f;
    }

    template <typename T> inline
    void Vec4<T>::operator*=(const T & f)
    {
        x() *= f;
        y() *= f;
        z() *= f;
        w() *= f;
    }

    template <typename T> inline
    void Vec4<T>::operator+=(const Vec4<T> & v)
    {
        x() += v.x();
        y() += v.y();
        z() += v.z();
        w() += v.w();
    }

    template <typename T> inline
    void Vec4<T>::operator-=(const Vec4<T> & v)
    {
        x() -= v.x();
        y() -= v.y();
        z() -= v.z();
        w() -= v.w();
    }

    template <typename T> inline
    Vec4<T> Vec4<T>::operator-() const
    {
        return Vec4<T>(-x(), -y(), -z(), -w());
    }

    template <typename T> inline
    const T& Vec4<T>::operator[](unsigned int index) const
    {
        return data[index];
    }

    template <typename T> inline
    T& Vec4<T>::operator[](unsigned int index)
    {
        return data[index];
    }

    //---------------------------------------------------------------------------
    //  Const Member Functions
    //---------------------------------------------------------------------------

    //Returns the length of this vector.
    template <typename T> inline
    T Vec4<T>::magnitude() const
    {
        return sqrt(x() * x() + y() * y() + z() * z() + w() * w());
    }

    //Returns the squared magnitude of this vector.
    template <typename T> inline
    T Vec4<T>::sqrtMagnitude() const
    {
        return x() * x() + y() * y() + z() * z() + w() * w();
    }

    //Returns the normalized vector with a length of 1.
    template <typename T> inline
    Vec4<T> Vec4<T>::normalized() const
    {
        T length = this->magnitude();
        if (length == 0.0f)
            return *this;
        return Vec4<T>(x() / length, y() / length, z() / length, w() / length);
    }

    //Standard dot-product between both vectors.
    template <typename T> inline
    T Vec4<T>::dot(const Vec4<T> & v) const
    {
        return x() * v.x() + y() * v.y() + z() * v.z() + w() * v.w();
    }

    //Calculates the angle between both vectors. Optimized for floating-point calculation (Using acosf). Returns 0 if one of both vectors is a zero vector.
    template <typename T> inline
    T Vec4<T>::angle(const Vec4<T> & v) const
    {
        T res = this->magnitude() * v.magnitude();
        if (res == 0.0f)
            return 0.0f;
        return acosf(this->dot(v) / res);
    }

    //Calculates the distance between both vectors.
    template <typename T> inline
    T Vec4<T>::distance(const Vec4<T> & v) const
    {
        return (*this - v).magnitude();
    }

    //Returns a vector that is made from the largest components of two vectors.
    template <typename T> inline
    Vec4<T> Vec4<T>::maxVec(const Vec4<T> & v) const
    {
        T x, y, z, w;
        this->x() > v.x() ? x = this->x() : x = v.x();
        this->y() > v.y() ? y = this->y() : y = v.y();
        this->z() > v.z() ? z = this->z() : z = v.z();
        this->w() > v.w() ? w = this->w() : w = v.w();
        return Vec4<T>(x, y, z, w);
    }

    //Returns a vector that is made from the smallest components of two vectors.
    template <typename T> inline
    Vec4<T> Vec4<T>::minVec(const Vec4<T> & v) const
    {
        T x, y, z, w;
        this->x() < v.x() ? x = this->x() : x = v.x();
        this->y() < v.y() ? y = this->y() : y = v.y();
        this->z() < v.z() ? z = this->z() : z = v.z();
        this->w() < v.w() ? w = this->w() : w = v.w();
        return Vec4<T>(x, y, z, w);
    }

    //Returns a copy of vector with its magnitude clamped to maxLength.
    template <typename T> inline
    Vec4<T> Vec4<T>::clampMagnitude(float maxLength) const
    {
        if (this->magnitude() < maxLength)
            return *this;
        return this->normalized() * maxLength;
    }

    //Return a new vector which is projected from this vector onto the given vector argument. 
    template <typename T> inline
    Vec4<T> Vec4<T>::project(const Vec4<T> & v)	const
    {
        T v_length = v.magnitude();
        if (v_length == 0.0f)
            return v; //Return zero vector
        return (this->dot(v) / (v_length * v_length)) * v;
    }

    //Spherically interpolates between two vectors. Lerp amount is clamped to 0 - 1.
    template <typename T> inline
    Vec4<T> Vec4<T>::slerp(const Vec4<T> & end, float percent) const
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
        Vec4<T> relativeVec = end - *this*dot;
        relativeVec.normalize();     // Orthonormal basis
                                     // The final result.
        return ((*this*cos(theta)) + (relativeVec*sin(theta)));
    }

    // Return the First-Value of this vector.
    template <typename T> inline
    const T& Vec4<T>::x() const
    {
        return data[0];
    }

    // Return the Second-Value of this vector.
    template <typename T> inline
    const T& Vec4<T>::y() const
    {
        return data[1];
    }

    // Return the Third-Value of this vector.
    template <typename T> inline
    const T& Vec4<T>::z() const
    {
        return data[2];
    }

    // Return the Fourth-Value of this vector.
    template <typename T> inline
    const T& Vec4<T>::w() const
    {
        return data[3];
    }

    // Return the First-Value of this vector.
    template <typename T> inline
    const T& Vec4<T>::r() const
    {
        return x();
    }

    // Return the Second-Value of this vector.
    template <typename T> inline
    const T& Vec4<T>::g() const
    {
        return y();
    }

    // Return the Third-Value of this vector.
    template <typename T> inline
    const T& Vec4<T>::b() const
    {
        return z();
    }

    // Return the Fourth-Value of this vector.
    template <typename T> inline
    const T& Vec4<T>::a() const
    {
        return w();
    }

    //Basically checks if the values are not FLT_MAX
    template <typename T> inline
    bool Vec4<T>::isValid() const
    {
        return x() != FLT_MAX && y() != FLT_MAX && z() != FLT_MAX && w() != FLT_MAX;
    }


    //---------------------------------------------------------------------------
    //  Nonconst Member Functions
    //---------------------------------------------------------------------------

    //Make this vector have a magnitude of 1.
    template <typename T> inline
    void Vec4<T>::normalize()
    {
        T length = this->magnitude();
        if (length == 0.0f)
            return;
        x() /= length;
        y() /= length;
        z() /= length;
        w() /= length;
    }

    //Set x, y, z and w to newX, newY, newZ and newW.
    template <typename T> inline
    void Vec4<T>::set(T newX, T newY, T newZ, T newW)
    {
        x() = newX;
        y() = newY;
        z() = newZ;
        w() = newW;
    }

    //Scale this vector by the amount of scale.
    template <typename T> inline
    void Vec4<T>::scale(T scale)
    {
        x() *= scale;
        y() *= scale;
        z() *= scale;
        w() *= scale;
    }

    //Scale this vector by the corresponding field of vector v.
    template <typename T> inline
    void Vec4<T>::scale(const Vec4<T> & v)
    {
        x() *= v.x();
        y() *= v.y();
        z() *= v.z();
        w() *= v.w();
    }

    //Moves the point in a straight line "maxDistanceDelta" towards the target point. Can't exceed the target. Negative Values for the delta argument moves the point away (No Limit!).
    template <typename T> inline
    void Vec4<T>::moveTowards(const Vec4<T> & target, float maxDistanceDelta)
    {
        if (this->distance(target) < maxDistanceDelta)
            *this = target;
        else
            *this += ((target - *this).clampMagnitude(maxDistanceDelta));
    }

    // Return the First-Value of this vector by reference.
    template <typename T> inline
    T& Vec4<T>::x()
    {
        return data[0];
    }

    // Return the Second-Value of this vector by reference.
    template <typename T> inline
    T& Vec4<T>::y()
    {
        return data[1];
    }

    // Return the Third-Value of this vector by reference.
    template <typename T> inline
    T& Vec4<T>::z()
    {
        return data[2];
    }

    // Return the Fourth-Value of this vector by reference.
    template <typename T> inline
    T& Vec4<T>::w()
    {
        return data[3];
    }

    // Return the First-Value of this vector by reference.
    template <typename T> inline
    T& Vec4<T>::r()
    {
        return x();
    }

    // Return the Second-Value of this vector by reference.
    template <typename T> inline
    T& Vec4<T>::g()
    {
        return y();
    }

    // Return the Third-Value of this vector by reference.
    template <typename T> inline
    T& Vec4<T>::b()
    {
        return z();
    }

    // Return the Fourth-Value of this vector by reference.
    template <typename T> inline
    T& Vec4<T>::a()
    {
        return w();
    }

}
#endif // _VEC4_H_

