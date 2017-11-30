/*
*  Point - Class header file.
*  Represents a point in a 3D-space.
*
*  Date:       11.04.2016
*  Creator:    Silvan Hau
*/
#ifndef _POINT_H_
#define _POINT_H_

#include "Vector3.h"

namespace math {

    template <typename T>
    class Point : public Vec3<T>
    {
    public:
        //Constructor
        explicit Point(T _x = 0, T _y = 0, T _z = 0) : Vec3<T>(_x,_y,_z) {}
        explicit Point(const Vec2<T> & v2) { x() = v2.x(); y() = v2.y(); z() = 0; }
        explicit Point(const Vec3<T> & v3) { x() = v3.x(); y() = v3.y(); z() = v3.z(); }
        explicit Point(const Vec4<T> & v4) { x() = v4.x(); y() = v4.y(); z() = v4.z(); }

        //Operators
        Point<T>     operator+           (const Vec3<T> &)           const;
        Point<T>     operator+           (const Point<T> &)          const;
        Point<T>     operator-           (const Vec3<T> &)           const;
        Point<T>     operator-           (const Point<T> &)          const;
        Point<T>     operator-           ()                          const;
        Point<T>     operator*           (T)                         const;
        Point<T>     operator/           (T)                         const;
    };

    template <typename T> inline
    Point<T> Point<T>::operator+ (const Vec3<T> &v) const
    {
        return Point<T>(x() + v.x(), y() + v.y(), z() + v.z());
    }

    template <typename T> inline
    Point<T> Point<T>::operator+(const Point<T> & v) const
    {
        return Point<T>(x() + v.x(), y() + v.y(), z() + v.z());
    }

    template <typename T> inline
    Point<T> Point<T>::operator- (const Vec3<T> &v) const
    {
        return Point<T>(x() - v.x(), y() - v.y(), z() - v.z());
    }

    template <typename T> inline
    Point<T> Point<T>::operator-(const Point<T> & v) const
    {
        return Point<T>(x() - v.x(), y() - v.y(), z() - v.z());
    }

    template <typename T> inline
    Point<T> Point<T>::operator-() const
    {
        return Point<T>(-x(), -y(), -z());
    }

    // "*" operator for point * scalar (member func) AND scalar * point (nonmember func)
    template <typename T> inline
    Point<T> Point<T>::operator*(T s) const
    {
        return Point<T>(x() * s, y() * s, z() * s);
    }
    template <typename T> inline
    Point<T> operator*(float s, const Point<T> & v)
    {
        return Point<T>(v.x() * s, v.y() * s, v.z() * s);
    }

    template <typename T> inline
    Point<T> Point<T>::operator/(T s) const
    {
        return Point<T>(x() / s, y() / s, z() / s);
    }
}

#endif