/*
*  Quaternion - Class header file.
*
*  Date:       26.03.2016
*  Creator:    Silvan Hau
*/
#ifndef _QUAT_H_
#define _QUAT_H_

//---------------------------------------------------------------------------
//  Forward Declarations
//---------------------------------------------------------------------------
template <typename T> class Vec3;
template <typename T> class Vec4;
template <typename T> class Mat3x3;
template <typename T> class Mat4x4;

namespace math
{
    //---------------------------------------------------------------------------
    //  Quaternion Class
    //---------------------------------------------------------------------------
    template <typename T>
    class Quaternion
    {
        T data[4];

    public:
        //Constructor
        explicit Quaternion(T x = 0, T y = 0, T z = 0, T w = 1) { data[0] = x; data[1] = y; data[2] = z; data[3] = w; }
        Quaternion(const Vec3<T> & axis, T radians);
        Quaternion(const Vec4<T> & v4) { x() = v4.x(); y() = v4.y(); z() = v4.z(); w() = v4.w(); }

        //Operators
        Quaternion<T>     operator*       (const Quaternion<T> &)           const;
        //Quaternion<T>     operator*       (const Vec3<T> &)                 const;
        Quaternion<T>     operator+       (const Quaternion<T> &)           const;
        Quaternion<T>     operator-       (const Quaternion<T> &)           const;
        bool              operator==      (const Quaternion<T> &)           const;
        bool              operator!=      (const Quaternion<T> &)           const;
        Vec3<T>           operator*       (const Vec3<T> &)                 const;
        const T&          operator[]      (unsigned int)                    const;
        Quaternion<T>     operator-       ()                                const;
        Quaternion<T>     operator*       (T scale)                         const;
        Quaternion<T>     operator/       (T scale)                         const;
        void              operator*=      (const Quaternion<T> &);
        T&                operator[]      (unsigned int);

        //Conversion functions
        explicit operator Mat3x3<T>() const { return this->toMatrix3x3(); }
        explicit operator Mat4x4<T>() const { return this->toMatrix4x4(); }

        //Const member functions
        Mat3x3<T>         toMatrix3x3     ()                                const;       //Converts the quaternion to a 3x3 rotation matrix.
        Mat4x4<T>         toMatrix4x4     ()                                const;       //Converts the quaternion to a 4x4 rotation matrix.
        Vec3<T>           toEulerAngles   ()                                const;       //Returns the quaternion as a vec3 in euler angles.
        T                 dot             (const Quaternion<T> &)           const;       //Standard dot product.
        Quaternion<T>     conjugate       ()                                const;       //Returns the conjugate. The conjugate is mostly equal the inverse if length = 1.
        Quaternion<T>     inversed        ()                                const;       //Returns the inverse. Same as conjugate if length = 1.
        Quaternion<T>     lerp            (const Quaternion<T> &, float)    const;       //Standard lerp. LerpFac will be clamped to 0 - 1. LerpFac = 0: returns called object, lerpFac = 1: returns given quaternion q.
        Quaternion<T>     lerpUnclamped   (const Quaternion<T> &, float)    const;       //Standard lerp. LerpFac = 0: returns called object, lerpFac = 1: returns given quaternion q.
        Quaternion<T>     slerp           (const Quaternion<T> &, float)    const;       //Spherical Interpolation. LerpFac will be clamped to 0 - 1. LerpFac = 0: returns called object, lerpFac = 1: returns given quaternion q.
        Quaternion<T>     slerpUnclamped  (const Quaternion<T> &, float)    const;       //Spherical Interpolation. LerpFac = 0: returns called object, lerpFac = 1: returns given quaternion q.
        Vec3<T>           getForward      ()                                const;       //Return a 3d-Vector representing the direction
        Vec3<T>           getBack         ()                                const;       //Return a 3d-Vector representing the direction
        Vec3<T>           getLeft         ()                                const;       //Return a 3d-Vector representing the direction
        Vec3<T>           getRight        ()                                const;       //Return a 3d-Vector representing the direction
        Vec3<T>           getUp           ()                                const;       //Return a 3d-Vector representing the direction
        Vec3<T>           getDown         ()                                const;       //Return a 3d-Vector representing the direction
        Quaternion<T>     normalized      ()                                const;       //Return a normalized Quaternion with a magnitude of 1.
        T                 x               ()                                const;       //Return the First-Value of this quaternion.
        T                 y               ()                                const;       //Return the Second-Value of this quaternion.
        T                 z               ()                                const;       //Return the Third-Value of this quaternion.
        T                 w               ()                                const;       //Return the Fourth-Value of this quaternion.

        //Nonconst member functions
        T&                x               ();                                            //Return the First-Value of this quaternion by reference.
        T&                y               ();                                            //Return the Second-Value of this quaternion by reference.
        T&                z               ();                                            //Return the Third-Value of this quaternion by reference.
        T&                w               ();                                            //Return the Fourth-Value of this quaternion by reference.

        T                  magnitude      ()     const;                                  //Returns the length of this quaternion.
        T                  sqrtMagnitude  ()     const;                                  //Returns the squared length of this quaternion.
        //Static members
        static Quaternion<T> identity;                                                   //Represents zero rotation.

        //Static functions
        static Quaternion<T> fromTwoVectors(const Vec3<T>&, const Vec3<T>&);             //Build a quaternion, which represents the shortest arc between those 2 vectors.
        static Quaternion<T> lookRotation (const Vec3<T>&, const Vec3<T>& = Vec3<T>::up);//Build a quaternion from a given forward and up vector
        static Quaternion<T> rotationX    (T);                                           //Build a quaternion which rotates radians about the x-axis.
        static Quaternion<T> rotationY    (T);                                           //Build a quaternion which rotates radians about the y-axis.
        static Quaternion<T> rotationZ    (T);                                           //Build a quaternion which rotates radians about the z-axis.
        static Quaternion<T> euler        (T, T, T);                                     //Build a quaternion from a given rotation about the x, y and z axis.

    private:
        friend class Random; //Allow Random using the "normalize" function to create a random Quaternion.

        //Const member functions

        //Nonconst member functions
        Quaternion<T>&     normalize();                                                  //Make this Quaternion have a magnitude of 1.

    };

    //---------------------------------------------------------------------------
    //  Static Member Initialization
    //---------------------------------------------------------------------------
    template <typename T> Quaternion<T> Quaternion<T>::identity = Quaternion<T>(0, 0, 0, 1);

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------
    template <typename T>
    Quaternion<T>::Quaternion(const Vec3<T> & axis, T radians)
    {
        T halfSine = sin(radians / 2);
        this->x() = axis.x() * halfSine;
        this->y() = axis.y() * halfSine;
        this->z() = axis.z() * halfSine;
        this->w() = cos(radians / 2);
    }

    //---------------------------------------------------------------------------
    //  Operators
    //---------------------------------------------------------------------------
    template <typename T> inline
    Quaternion<T> Quaternion<T>::operator*(const Quaternion<T> & q) const
    {
        return Quaternion<T>(
             (x() * q.w()) + (y() * q.z()) - (z() * q.y()) + (w() * q.x()),
            -(x() * q.z()) + (y() * q.w()) + (z() * q.x()) + (w() * q.y()),
             (x() * q.y()) - (y() * q.x()) + (z() * q.w()) + (w() * q.z()),
            -(x() * q.x()) - (y() * q.y()) - (z() * q.z()) + (w() * q.w())
            );
    }

    template <typename T> inline
    Quaternion<T> Quaternion<T>::operator+(const Quaternion<T> & q) const
    {
        return Quaternion<T>(x() + q.x(), y() + q.y(), z() + q.z(), w() + q.w());
    }

    template <typename T> inline
    Quaternion<T> Quaternion<T>::operator-(const Quaternion<T> & q) const
    {
        return Quaternion<T>(x() - q.x(), y() - q.y(), z() - q.z(), w() - q.w());
    }

 /* template <typename T> inline
    Quaternion<T> Quaternion<T>::operator*(const Vec3<T> & r) const
    {
        return Quaternion<T>(
             w * r.x + y * r.z - z * r.y,
             w * r.y + z * r.x - x * r.z,
             w * r.z + x * r.y - y * r.x,
            -x * r.x - y * r.y - z * r-z
            );
    }*/

    template <typename T> inline
    bool Quaternion<T>::operator==(const Quaternion<T> & q) const
    {
        return x() == q.x() && y() == q.y() && z() == q.z() && w() == q.w();
    }

    template <typename T> inline
    bool Quaternion<T>::operator!=(const Quaternion<T> & q) const
    {
        return !(*this == q);
    }

    template <typename T> inline
    Vec3<T> Quaternion<T>::operator* (const Vec3<T> & v) const
    {
        Quaternion<T> result = *this * Quaternion<T>(v.x(), v.y(), v.z(), 0) * this->conjugate();
        return Vec3<T>(result.x(), result.y(), result.z());
    }

    template <typename T> inline
    const T& Quaternion<T>::operator[](unsigned int index) const
    {
        return data[index];
    }

    template <typename T> inline
    Quaternion<T> Quaternion<T>::operator-() const
    {
        return Quaternion<T>(-x(), -y(), -z(), -w());
    }

    template <typename T> inline
    Quaternion<T> Quaternion<T>::operator* (T scale) const
    {
        return Quaternion<T>(x() * scale, y() * scale, z() * scale, w() * scale);
    }

    template <typename T> inline
    Quaternion<T> Quaternion<T>::operator/ (T scale) const
    {
        return Quaternion<T>(x() / scale, y() / scale, z() / scale, w() / scale);
    }

    template <typename T> inline
    void Quaternion<T>::operator*= (const Quaternion<T> &q)
    {
        T x, y, z, w;
        x = this->x(); y = this->y(); z = this->z(); w = this->w();
        this->x() =  (q.x() * w) + (q.y() * z) - (q.z() * y) + (q.w() * x);
        this->y() = -(q.x() * z) + (q.y() * w) + (q.z() * x) + (q.w() * y);
        this->z() =  (q.x() * y) - (q.y() * x) + (q.z() * w) + (q.w() * z);
        this->w() = -(q.x() * x) - (q.y() * y) - (q.z() * z) + (q.w() * w);
    }

    template <typename T> inline
    T& Quaternion<T>::operator[](unsigned int index)
    {
        return data[index];
    }

    //---------------------------------------------------------------------------
    // Const member functions
    //---------------------------------------------------------------------------

    //Returns the length of this quaternion.
    template <typename T> inline
    T Quaternion<T>::magnitude() const
    {
        return sqrt(x() * x() + y() * y() + z() * z() + w() * w());
    }

    //Returns the squared length of this quaternion.
    template <typename T> inline
    T Quaternion<T>::sqrtMagnitude() const
    {
        return x() * x() + y() * y() + z() * z() + w() * w();
    }

    //Converts the quaternion to a 3x3 rotation matrix.
    template <typename T> inline
    Mat3x3<T> Quaternion<T>::toMatrix3x3() const
    {
        return Mat3x3<T>(1 - 2 * (y()*y() + z()*z()),     2 * (x()*y() - w()*z()),     2 * (x()*z() + w()*y()),
                             2 * (x()*y() + w()*z()), 1 - 2 * (x()*x() + z()*z()),     2 * (y()*z() - w()*x()),
                             2 * (x()*z() - w()*y()),     2 * (y()*z() + w()*x()), 1 - 2 * (x()*x() + y()*y()));
    }

    //Converts the quaternion to a 4x4 rotation matrix.
    template <typename T> inline
    Mat4x4<T> Quaternion<T>::toMatrix4x4() const
    {
        return Mat4x4<T>(1 - 2 * (y()*y() + z()*z()),     2 * (x()*y() - w()*z()),     2 * (x()*z() + w()*y()), 0,
                             2 * (x()*y() + w()*z()), 1 - 2 * (x()*x() + z()*z()),     2 * (y()*z() - w()*x()), 0,
                             2 * (x()*z() - w()*y()),     2 * (y()*z() + w()*x()), 1 - 2 * (x()*x() + y()*y()), 0,
                                                   0,                           0,                           0, 1);
    }

    //Returns the quaternion as a vec3 in euler angles. (Algorithmus from http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/)
    template <typename T> inline
    Vec3<T> Quaternion<T>::toEulerAngles() const
    {
        Vec3<T> result;

        T test = x()*y() + z()*w();
        if (test > 0.499) { // singularity at north pole
            result.x() = 0;
            result.y() = 2 * atan2(x(), w());
            result.z() = Mathf::PI_F / 2;
        }
        else if (test < -0.499) { // singularity at south pole
            result.x() = 0;
            result.y() = -2 * atan2(x(), w());
            result.z() = -Mathf::PI_F / 2;
        }
        else {
            T sqz = z()*z();
            result.x() = atan2(2 * x()*w() - 2 * y()*z(), 1 - 2 * x()*x() - 2 * sqz);
            result.y() = atan2(2 * y()*w() - 2 * x()*z(), 1 - 2 * y()*y() - 2 * sqz);
            result.z() = asin(2 * test);
        }

        result.x() = Mathf::rad2Deg(result.x());
        result.y() = Mathf::rad2Deg(result.y());
        result.z() = Mathf::rad2Deg(result.z());

        return result;
    }

    //Standard dot product.
    template <typename T> inline
    T Quaternion<T>::dot(const Quaternion<T> & q) const
    {
        return x() * q.x() + y() * q.y() + z() * q.z() + w() * q.w();
    }

    //Returns the conjugate. The conjugate is mostly equal the inverse if length = 1.
    template <typename T> inline
    Quaternion<T> Quaternion<T>::conjugate() const
    {
        return Quaternion<T>(-x(), -y(), -z(), w());
    }

    //Returns the inverse. Same as conjugate if length = 1.
    template <typename T> inline
    Quaternion<T> Quaternion<T>::inversed() const
    {
        T length = this->sqrtMagnitude();
        return Quaternion<T>(-x() / length, -y() / length, -z() / length, w() / length);
    }

    //Standard lerp. LerpFac will be clamped to 0 - 1. LerpFac = 0: returns called object, lerpFac = 1: returns given quaternion q.
    template <typename T> inline
    Quaternion<T> Quaternion<T>::lerp(const Quaternion<T> & q, float lerpFac) const
    {
        Mathf::clamp(lerpFac, 0.0f, 1.0f);
        return lerpUnclamped(q, lerpFac);
    }

    //Standard lerp. LerpFac = 0: returns called object, lerpFac = 1: returns given quaternion q.
    template <typename T> inline
    Quaternion<T> Quaternion<T>::lerpUnclamped(const Quaternion<T> & q, float lerpFac) const
    {
        T oneMinusLerpFac = 1 - lerpFac;
        return Quaternion<T>(oneMinusLerpFac * x() + lerpFac * q.x(),
                             oneMinusLerpFac * y() + lerpFac * q.y(),
                             oneMinusLerpFac * z() + lerpFac * q.z(),
                             oneMinusLerpFac * w() + lerpFac * q.w()).normalize();
    }

    //Spherical Interpolation. LerpFac will be clamped to 0 - 1. LerpFac = 0: returns called object, lerpFac = 1: returns given quaternion q.
    template <typename T> inline
    Quaternion<T> Quaternion<T>::slerp(const Quaternion<T> & q, float lerpFac) const
    {
        Mathf::clamp(lerpFac, 0.0f, 1.0f);
        return slerpUnclamped(q, lerpFac);
    }

    //Spherical Interpolation. LerpFac = 0: returns called object, lerpFac = 1: returns given quaternion q.
    //Algorthmus from http://willperone.net/Code/quaternion.php
    template <typename T> inline
    Quaternion<T> Quaternion<T>::slerpUnclamped(const Quaternion<T> & q, float lerpFac) const
    {
        Quaternion<T> q2 = q;
        float dot = this->dot(q);

        // if (dot < 0), q1 and q2 are more than 90 degrees apart,
        // so we can invert one to reduce spinning.
        if (dot < 0)
        {
            dot = -dot;
            q2 = -q;
        }

        if (dot < 0.95f)
        {
            float angle = acosf(dot);
            return ((*this) * sinf(angle*(1 - lerpFac)) + q2 * sinf(angle*lerpFac)) / sinf(angle);
        }
        else // if the angle is small, use linear interpolation
            return this->lerp(q2, lerpFac);
    }

    //Return the forward vector rotated by this quaternion
    template <typename T> inline
    Vec3<T> Quaternion<T>::getForward() const
    {
        //return Vec3<T>(-2.0f * (x*z - w*y), -2.0f * (y*z + w*x), -(1.0f - 2.0f * (x*x + y*y)));
        return  *this * Vec3<T>::back;
    }
    
    //Return the back vector rotated by this quaternion
    template <typename T> inline
    Vec3<T> Quaternion<T>::getBack() const
    {
        //return Vec3<T>(2.0f * (x*z - w*y), 2.0f * (y*z + w*x), 1.0f - 2.0f * (x*x + y*y));
        return  *this * Vec3<T>::forward;
    }

    //Return the left vector rotated by this quaternion
    template <typename T> inline
    Vec3<T> Quaternion<T>::getLeft() const
    {
        //return Vec3<T>(-(1.0f - 2.0f * (y*y + z*z)), -2.0f * (x*y - w*z), -2.0f * (x*z + w*y));
        return  *this * Vec3<T>::right;
    }

    //Return the right vector rotated by this quaternion
    template <typename T> inline
    Vec3<T> Quaternion<T>::getRight() const
    {
        //return Vec3<T>(1.0f - 2.0f * (y*y + z*z), 2.0f * (x*y - w*z), 2.0f * (x*z + w*y));
        return  *this * Vec3<T>::left;
    }

    //Return the up vector rotated by this quaternion
    template <typename T> inline
    Vec3<T> Quaternion<T>::getUp() const
    {
        //return Vec3<T>(2.0f * (x*y + w*z), 1.0f - 2.0f * (x*x + z*z), 2.0f * (y*z - w*x));
        return  *this * Vec3<T>::up;
    }

    //Return the down vector rotated by this quaternion
    template <typename T> inline
    Vec3<T> Quaternion<T>::getDown() const
    {
        //return Vec3<T>(-2.0f * (x*y + w*z), -(1.0f - 2.0f * (x*x + z*z)), -2.0f * (y*z - w*x));
        return  *this * Vec3<T>::down;
    }

    //Make this vector have a magnitude of 1.
    template <typename T> inline
    Quaternion<T> Quaternion<T>::normalized() const
    {
        T length = this->magnitude();
        return Quaternion<T>(x() / length, y() / length, z() / length, w() / length);
    }


    // Return the First-Value of this Quaternion.
    template <typename T> inline
    T Quaternion<T>::x() const
    {
        return data[0];
    }

    // Return the Second-Value of this Quaternion.
    template <typename T> inline
    T Quaternion<T>::y() const
    {
        return data[1];
    }

    // Return the Third-Value of this Quaternion.
    template <typename T> inline
    T Quaternion<T>::z() const
    {
        return data[2];
    }

    // Return the Fourth-Value of this Quaternion.
    template <typename T> inline
    T Quaternion<T>::w() const
    {
        return data[3];
    }

    //---------------------------------------------------------------------------
    //  Nonconst Member Functions
    //---------------------------------------------------------------------------

    //Make this vector have a magnitude of 1.
    template <typename T> inline
    Quaternion<T>& Quaternion<T>::normalize()
    {
        T length = this->magnitude();

        x() /= length;
        y() /= length;
        z() /= length;
        w() /= length;

        return *this;
    }

    // Return the First-Value of this Quaternion by reference.
    template <typename T> inline
    T& Quaternion<T>::x()
    {
        return data[0];
    }

    // Return the Second-Value of this Quaternion by reference.
    template <typename T> inline
    T& Quaternion<T>::y()
    {
        return data[1];
    }

    // Return the Third-Value of this Quaternion by reference.
    template <typename T> inline
    T& Quaternion<T>::z()
    {
        return data[2];
    }

    // Return the Fourth-Value of this Quaternion by reference.
    template <typename T> inline
    T& Quaternion<T>::w()
    {
        return data[3];
    }

    //---------------------------------------------------------------------------
    //  Static Functions
    //---------------------------------------------------------------------------

    //Build a quaternion which rotates radians about the x-axis.
    template <typename T> inline
    Quaternion<T> Quaternion<T>::rotationX(T radians)
    {
        T halfSine = sin(radians / 2);
        return Quaternion<T>(halfSine, 0, 0, cos(radians / 2));
    }

    //Build a quaternion which rotates radians about the y-axis.
    template <typename T> inline
    Quaternion<T> Quaternion<T>::rotationY(T radians)
    {
        T halfSine = sin(radians / 2);
        return Quaternion<T>(0, halfSine, 0, cos(radians / 2));
    }

    //Build a quaternion which rotates radians about the z-axis.
    template <typename T> inline
    Quaternion<T> Quaternion<T>::rotationZ(T radians)
    {
        T halfSine = sin(radians / 2);
        return Quaternion<T>(0, 0, halfSine, cos(radians / 2));
    }

    //Build a quaternion from a given forward and up vector
    template <typename T> inline
    Quaternion<T> Quaternion<T>::lookRotation(const Vec3<T>& forward, const Vec3<T>& up)
    {
        Mat4x4<T> lookAtMatrix = Mat4x4<T>::lookAt(Vec3<T>(0,0,0), forward, up);
        return lookAtMatrix.toQuaternion().conjugate();
    }

    //Build a quaternion from a given rotation about the x, y and z axis.
    template <typename T> inline
    Quaternion<T> Quaternion<T>::euler(T xDegree, T yDegree, T zDegree)
    {
        return Vec3<T>(xDegree, yDegree, zDegree).toQuaternion();
    }

    //Build a quaternion, which represents the shortest arc between those 2 vectors.
    template <typename T> inline
    Quaternion<T> Quaternion<T>::fromTwoVectors(const Vec3<T>& u, const Vec3<T>& v)
    {
        float norm_u_norm_v = sqrt(u.dot(u) * v.dot(v));
        float real_part = norm_u_norm_v + u.dot(v);
        Vec3f w;

        if (real_part < 1.e-6f * norm_u_norm_v)
        {
            /* If u and v are exactly opposite, rotate 180 degrees
            * around an arbitrary orthogonal axis. Axis normalisation
            * can happen later, when we normalise the quaternion. */
            real_part = 0.0f;
            w = abs(u.x()) > abs(u.z()) ? Vec3f(-u.y(), u.x(), 0.f)
                : Vec3f(0.f, -u.z(), u.y());
        }
        else
        {
            /* Otherwise, build quaternion the standard way. */
            w = u.cross(v);
        }

        return Quatf(w.x(), w.y(), w.z(), real_part).normalized();
    }

}
#endif