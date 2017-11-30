/*
*  Matrix4x4 - Class header file.
*
*  Date:       23.03.2016
*  Creator:    Silvan Hau
*/
#ifndef _MAT4_H_
#define _MAT4_H_

#include "util.h"
#include "Vector3.h"                /* Rotation about an arbitrary axis --- static Mat4x4<T> rotationArbitrary(const Vec3<T> &, float); */
#include "Vector4.h"                /* Rows interpreted as Vector-4 with [] operator --- Vec4<T>& operator[] (unsigned int);            */
#include "Quaternion.h"             /* Convert rotation matrix to quaternion + build a transformation matrix                            */
#include "Point.h"                  /* Mat4x4 * Point                                                                                   */
#include "Matrix3x3.h"              /* Convert to Matrix3x3                                                                             */

//---------------------------------------------------------------------------
//  Forward Declarations
//---------------------------------------------------------------------------
template <typename T> class Vec3;
template <typename T> class Vec4;
template <typename T> class Quaternion;
template <typename T> class Mat3x3;

namespace math
{
    //---------------------------------------------------------------------------
    //  4x4 - Matrix Class
    //---------------------------------------------------------------------------
    template <typename T>
    class Mat4x4
    {
        // [row + col * 4]
        T elem[16];

    public:
        //Constructors
        Mat4x4(T value = 0)
        {
            elem[0 + 0 * 4] = elem[0 + 1 * 4] = elem[0 + 2 * 4] = elem[0 + 3 * 4] = value;
            elem[1 + 0 * 4] = elem[1 + 1 * 4] = elem[1 + 2 * 4] = elem[1 + 3 * 4] = value;
            elem[2 + 0 * 4] = elem[2 + 1 * 4] = elem[2 + 2 * 4] = elem[2 + 3 * 4] = value;
            elem[3 + 0 * 4] = elem[3 + 1 * 4] = elem[3 + 2 * 4] = elem[3 + 3 * 4] = value;
        }

        Mat4x4(
            T i11, T i12, T i13, T i14,
            T i21, T i22, T i23, T i24,
            T i31, T i32, T i33, T i34,
            T i41, T i42, T i43, T i44
            )
        {
            elem[0 + 0 * 4] = i11; elem[0 + 1 * 4] = i12; elem[0 + 2 * 4] = i13; elem[0 + 3 * 4] = i14;
            elem[1 + 0 * 4] = i21; elem[1 + 1 * 4] = i22; elem[1 + 2 * 4] = i23; elem[1 + 3 * 4] = i24;
            elem[2 + 0 * 4] = i31; elem[2 + 1 * 4] = i32; elem[2 + 2 * 4] = i33; elem[2 + 3 * 4] = i34;
            elem[3 + 0 * 4] = i41; elem[3 + 1 * 4] = i42; elem[3 + 2 * 4] = i43; elem[3 + 3 * 4] = i44;
        }

        //Conversion functions
        explicit operator Mat3x3<T>() const { return Mat3x3<T>(elem[0], elem[4], elem[8],
                                                               elem[1], elem[5], elem[9],
                                                               elem[2], elem[6], elem[10]);}

        //Operators
        Mat4x4<T>       operator+           (const Mat4x4<T> &)    const;
        Mat4x4<T>       operator-           (const Mat4x4<T> &)    const;
        Mat4x4<T>       operator*           (const Mat4x4<T> &)    const;
        Mat4x4<T>       operator*           (T)                    const;
        Vec3<T>         operator*           (const Vec3<T> &)      const;
        Point<T>        operator*           (const Point<T> & p)   const;
        Vec4<T>         operator*           (const Vec4<T> &)      const;
        Mat4x4<T>       operator/           (T)                    const;
        Mat4x4<T>       operator-           ()                     const;
        bool            operator==          (const Mat4x4<T> &)    const;
        bool            operator!=          (const Mat4x4<T> &)    const;
        const Vec4<T>&  operator[]          (unsigned int)         const;
        Vec4<T>&        operator[]          (unsigned int);
        void            operator+=          (const Mat4x4<T> &);
        void            operator-=          (const Mat4x4<T> &);
        void            operator*=          (const Mat4x4<T> &);

        //Const member functions
        Mat4x4<T>       multiply            (const Mat4x4<T> &m)   const { return *this * m; } //Standard matrix-multiplication (uses overloaded "*" operator)
        Mat4x4<T>       transposed          ()                     const;                      //Returns a new transposed matrix from this one.
        T               determinant         ()                     const;                      //Returns the determinant for this matrix.
        Mat4x4<T>       inversed            ()                     const;                      //Returns a new inversed matrix from this one.
        Mat4x4<T>       inversedRotation    ()                     const;                      //Returns a new inversed rotation matrix from this one.
        Mat4x4<T>       inversedTranslation ()                     const;                      //Returns a new inversed translation matrix from this one.
        Mat4x4<T>       inversedScale       ()                     const;                      //Returns a new inversed scale matrix from this one.
        Vec4<T>         getColumn           (unsigned int)         const;                      //Returns the i-th colum of the matrix.
        Vec4<T>         getRow              (unsigned int)         const;                      //Returns the i-th row of the matrix.	
        Vec3<T>         multiplyPoint       (const Vec3<T> &)      const;                      //Transforms a position by this matrix
        Vec3<T>         multiplyDirection   (const Vec3<T> &)      const;                      //Transforms a direction by this matrix.
        Quaternion<T>   toQuaternion        ()                     const;                      //Returns an quaternion representing the rotation from this matrix.
        Vec3<T>         getTranslation      ()                     const;                      //Return an Vec3 representing the translation from this matrix.
        Vec3<T>         getScale            ()                     const;                      //Return an Vec3 representing the scale from this matrix.
        Quaternion<T>   getRotation         ()                     const;                      //Return an Quaternion representing the rotation from this matrix.
        Quaternion<T>   getRotation         (const Vec3<T> &)      const;                      //Return an Quaternion representing the rotation from this matrix.


        //Nonconst member functions
        Mat4x4<T>&      transpose           ();                                                //Transpose this matrix and returns it as a reference.
        Mat4x4<T>&      inverse             ();                                                //Inverse this matrix and returns it as a reference.
        void            set                 (unsigned int row, unsigned int col, T elem);      //Set a specific element

        //Static members
        static const Mat4x4<T> identity;                                                       //The identity matrix
        static const Mat4x4<T> zero;                                                           //The zero matrix

        //Static member functions
        static Mat4x4<T> scale              (T);                                                                       //Creates a new uniformed scale matrix.
        static Mat4x4<T> scale              (T, T, T);                                                                 //Creates a new nonuniformed scale matrix.
        static Mat4x4<T> scale              (const Vec3<T> & scale);                                                   //Creates a new nonuniformed scale matrix.
        static Mat4x4<T> translation        (T, T, T);                                                                 //Creates a new translation matrix.
        static Mat4x4<T> translation        (const Vec3<T> & trans);                                                   //Creates a new translation matrix.
        static Mat4x4<T> trs                (const Vec3<T> & trans, const Quaternion<T> & q, const Vec3<T> & scale);   //Creates a transformation matrix out of a translaton, rotation and scaling.
        static Mat4x4<T> initRotation       (const Vec3<T> &, const Vec3<T> &);                                        //Creates a rotation matrix from a forward and up vector
        static Mat4x4<T> initRotation       (const Vec3<T> &, const Vec3<T> &, const Vec3<T> &);                       //Creates a rotation matrix from a forward, up and right vector
        static Mat4x4<T> rotationX          (float);                                                                   //Creates a new matrix, which rotates about the X-axis.
        static Mat4x4<T> rotationY          (float);                                                                   //Creates a new matrix, which rotates about the Y-axis.
        static Mat4x4<T> rotationZ          (float);                                                                   //Creates a new matrix, which rotates about the Z-axis.
        static Mat4x4<T> rotationArbitrary  (const Vec3<T> &, float);                                                  //Creates a new matrix, which rotates about any arbitrary axis, given by a vec3-argument and an angle.
        static Mat4x4<T> lookAt             (const Vec3<T> &pos, const Vec3<T> &target, const Vec3<T> &up);            //Creates a new matrix, which looks at the target.
        static Mat4x4<T> view               (const Vec3<T> &cam, const Vec3<T> &forward, const Vec3<T> &up);           //Creates a new view matrix.

        static Mat4x4<T> ortho              (T left, T right, T bottom, T top, T _near, T _far);                       //Creates an orthographic projection matrix.
        static Mat4x4<T> ortho2             (T left, T right, T bottom, T top, T _near, T _far);                       //Creates an orthographic projection matrix.
        static Mat4x4<T> perspective        (T fov, T aspecRatio, T _near, T _far);                                    //Creates an perspective projection matrix.
    };

    //---------------------------------------------------------------------------
    //  Static Member Initialization
    //---------------------------------------------------------------------------
    template <typename T> const Mat4x4<T> Mat4x4<T>::identity = Mat4x4<T>(1, 0, 0, 0,
                                                                          0, 1, 0, 0,
                                                                          0, 0, 1, 0,
                                                                          0, 0, 0, 1);

    template <typename T> const Mat4x4<T> Mat4x4<T>::zero     = Mat4x4<T>(0, 0, 0, 0,
                                                                          0, 0, 0, 0,
                                                                          0, 0, 0, 0,
                                                                          0, 0, 0, 0);

    //---------------------------------------------------------------------------
    //  Operator Overloading
    //---------------------------------------------------------------------------
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::operator+(const Mat4x4<T> & m) const
    {
        return Mat4x4<T>(elem[0] + m.elem[0], elem[4] + m.elem[4],  elem[8] +  m.elem[8], elem[12] + m.elem[12],
                         elem[1] + m.elem[1], elem[5] + m.elem[5],  elem[9] +  m.elem[9], elem[13] + m.elem[13],
                         elem[2] + m.elem[2], elem[6] + m.elem[6], elem[10] + m.elem[10], elem[14] + m.elem[14],
                         elem[3] + m.elem[3], elem[7] + m.elem[7], elem[11] + m.elem[11], elem[15] + m.elem[15]);
    }

    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::operator-(const Mat4x4<T> & m) const
    {
        return Mat4x4<T>(elem[0] - m.elem[0], elem[4] - m.elem[4],  elem[8] -  m.elem[8], elem[12] - m.elem[12],
                         elem[1] - m.elem[1], elem[5] - m.elem[5],  elem[9] -  m.elem[9], elem[13] - m.elem[13],
                         elem[2] - m.elem[2], elem[6] - m.elem[6], elem[10] - m.elem[10], elem[14] - m.elem[14],
                         elem[3] - m.elem[3], elem[7] - m.elem[7], elem[11] - m.elem[11], elem[15] - m.elem[15]);
    }

    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::operator*(const Mat4x4<T> & m) const
    {
        return Mat4x4<T>(
            elem[0] * m.elem[0] + elem[4] * m.elem[1] +  elem[8] * m.elem[2] + elem[12] * m.elem[3], elem[0] * m.elem[4] + elem[4] * m.elem[5] +  elem[8] * m.elem[6] + elem[12] * m.elem[7], elem[0] * m.elem[8] + elem[4] * m.elem[9] +  elem[8] * m.elem[10] + elem[12] * m.elem[11], elem[0] * m.elem[12] + elem[4] * m.elem[13] +  elem[8] * m.elem[14] + elem[12] * m.elem[15],
            elem[1] * m.elem[0] + elem[5] * m.elem[1] +  elem[9] * m.elem[2] + elem[13] * m.elem[3], elem[1] * m.elem[4] + elem[5] * m.elem[5] +  elem[9] * m.elem[6] + elem[13] * m.elem[7], elem[1] * m.elem[8] + elem[5] * m.elem[9] +  elem[9] * m.elem[10] + elem[13] * m.elem[11], elem[1] * m.elem[12] + elem[5] * m.elem[13] +  elem[9] * m.elem[14] + elem[13] * m.elem[15],
            elem[2] * m.elem[0] + elem[6] * m.elem[1] + elem[10] * m.elem[2] + elem[14] * m.elem[3], elem[2] * m.elem[4] + elem[6] * m.elem[5] + elem[10] * m.elem[6] + elem[14] * m.elem[7], elem[2] * m.elem[8] + elem[6] * m.elem[9] + elem[10] * m.elem[10] + elem[14] * m.elem[11], elem[2] * m.elem[12] + elem[6] * m.elem[13] + elem[10] * m.elem[14] + elem[14] * m.elem[15],
            elem[3] * m.elem[0] + elem[7] * m.elem[1] + elem[11] * m.elem[2] + elem[15] * m.elem[3], elem[3] * m.elem[4] + elem[7] * m.elem[5] + elem[11] * m.elem[6] + elem[15] * m.elem[7], elem[3] * m.elem[8] + elem[7] * m.elem[9] + elem[11] * m.elem[10] + elem[15] * m.elem[11], elem[3] * m.elem[12] + elem[7] * m.elem[13] + elem[11] * m.elem[14] + elem[15] * m.elem[15]
            );
    }

    // "*" operator for mat * scalar (member func) AND scalar * mat (nonmember func)
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::operator*(T s) const
    {
        return Mat4x4<T>(elem[0] * s, elem[4] * s,  elem[8] * s, elem[12] * s,
                         elem[1] * s, elem[5] * s,  elem[9] * s, elem[13] * s,
                         elem[2] * s, elem[6] * s, elem[10] * s, elem[14] * s,
                         elem[3] * s, elem[7] * s, elem[11] * s, elem[15] * s);
    }
    template <typename T, typename T2> inline
    Mat4x4<T> operator*(T2 s, const Mat4x4<T> & m)
    {
        return m * s;
    }

    /* Multiplies the matrix with the given vector (DIRECTION) --> ignores translation part */
    template <typename T>
    Vec3<T> Mat4x4<T>::operator* (const Vec3<T> & v) const
    {
    	Vec4<T> vec4 = Vec4<T>(v.x(), v.y(), v.z(), 0);
    	return static_cast<Vec3<T>>(*this * vec4); //Use overloaded Matrix4x4 * Vector4 operator (see below)
    }

    template <typename T>
    Point<T> Mat4x4<T>::operator* (const Point<T> & p) const
    {
        Vec4<T> vec4 = Vec4<T>(p.x(), p.y(), p.z(), 1);
        return static_cast<Point<T>>(*this * vec4); //Use overloaded Matrix4x4 * Vector4 operator (see below)
    }

    template <typename T> inline
    Vec4<T> Mat4x4<T>::operator* (const Vec4<T> & v) const
    {
        return Vec4<T>(elem[0] * v[0] + elem[4] * v[1] +  elem[8] * v[2] + elem[12] * v[3],
                       elem[1] * v[0] + elem[5] * v[1] +  elem[9] * v[2] + elem[13] * v[3],
                       elem[2] * v[0] + elem[6] * v[1] + elem[10] * v[2] + elem[14] * v[3],
                       elem[3] * v[0] + elem[7] * v[1] + elem[11] * v[2] + elem[15] * v[3]);
    }

    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::operator/(T s) const
    {
        return Mat4x4<T>(elem[0] / s, elem[4] / s,  elem[8] / s, elem[12] / s,
                         elem[1] / s, elem[5] / s,  elem[9] / s, elem[13] / s,
                         elem[2] / s, elem[6] / s, elem[10] / s, elem[14] / s,
                         elem[3] / s, elem[7] / s, elem[11] / s, elem[15] / s);
    }

    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::operator-() const
    {
        return Mat4x4<T>(-elem[0], -elem[4],  -elem[8], -elem[12],
                         -elem[1], -elem[5],  -elem[9], -elem[13],
                         -elem[2], -elem[6], -elem[10], -elem[14],
                         -elem[3], -elem[7], -elem[11], -elem[15]);
    }

    template <typename T> inline
    bool Mat4x4<T>::operator== (const Mat4x4<T> & m) const
    {
        return  elem[0] == m.elem[0] && elem[4] == m.elem[4] &&  elem[8] ==  m.elem[8] && elem[12] == m.elem[12] &&
                elem[1] == m.elem[1] && elem[5] == m.elem[5] &&  elem[9] ==  m.elem[9] && elem[13] == m.elem[13] &&
                elem[2] == m.elem[2] && elem[6] == m.elem[6] && elem[10] == m.elem[10] && elem[14] == m.elem[14] &&
                elem[3] == m.elem[3] && elem[7] == m.elem[7] && elem[11] == m.elem[11] && elem[15] == m.elem[15];
    }

    template <typename T> inline
    bool Mat4x4<T>::operator!=(const Mat4x4<T> & m) const
    {
        return !(*this == m);
    }

    template <typename T> inline
    void Mat4x4<T>::operator+= (const Mat4x4<T> & m)
    {
        elem[0] += m.elem[0]; elem[4] += m.elem[4];  elem[8] +=  m.elem[8]; elem[12] += m.elem[12];
        elem[1] += m.elem[1]; elem[5] += m.elem[5];  elem[9] +=  m.elem[9]; elem[13] += m.elem[13];
        elem[2] += m.elem[2]; elem[6] += m.elem[6]; elem[10] += m.elem[10]; elem[14] += m.elem[14];
        elem[3] += m.elem[3]; elem[7] += m.elem[7]; elem[11] += m.elem[11]; elem[15] += m.elem[15];
    }

    template <typename T> inline
    void Mat4x4<T>::operator-= (const Mat4x4<T> & m)
    {
        elem[0] -= m.elem[0]; elem[4] -= m.elem[4];  elem[8] -=  m.elem[8]; elem[12] -= m.elem[12];
        elem[1] -= m.elem[1]; elem[5] -= m.elem[5];  elem[9] -=  m.elem[9]; elem[13] -= m.elem[13];
        elem[2] -= m.elem[2]; elem[6] -= m.elem[6]; elem[10] -= m.elem[10]; elem[14] -= m.elem[14];
        elem[3] -= m.elem[3]; elem[7] -= m.elem[7]; elem[11] -= m.elem[11]; elem[15] -= m.elem[15];
    }

    template <typename T> inline
    void Mat4x4<T>::operator*= (const Mat4x4<T> & m)
    {
        *this = *this * m;
    }

    template <typename T> inline
    const Vec4<T>& Mat4x4<T>::operator[](unsigned int index) const
    {
        return reinterpret_cast<const Vec4<T>*>(this)[index];
    }

    template <typename T> inline
    Vec4<T>& Mat4x4<T>::operator[](unsigned int index)
    {
        return reinterpret_cast<Vec4<T>*>(this)[index];
    }

    //---------------------------------------------------------------------------
    //  Const member functions
    //---------------------------------------------------------------------------

    //Returns a new transposed matrix from this one.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::transposed() const
    {
        return Mat4x4<T>( elem[0],  elem[1],  elem[2],  elem[3],
                          elem[4],  elem[5],  elem[6],  elem[7],
                          elem[8],  elem[9], elem[10], elem[11],
                         elem[12], elem[13], elem[14], elem[15]);
    }

    //Returns the determinant for this matrix.
    template <typename T> inline
    T Mat4x4<T>::determinant() const
    {
        return
            elem[12] * elem[9] *  elem[6] *  elem[3] - elem[8] * elem[13] *  elem[6] *  elem[3] -
            elem[12] * elem[5] * elem[10] *  elem[3] + elem[4] * elem[13] * elem[10] *  elem[3] +
             elem[8] * elem[5] * elem[14] *  elem[3] - elem[4] *  elem[9] * elem[14] *  elem[3] -
            elem[12] * elem[9] *  elem[2] *  elem[7] + elem[8] * elem[13] *  elem[2] *  elem[7] +
            elem[12] * elem[1] * elem[10] *  elem[7] - elem[0] * elem[13] * elem[10] *  elem[7] -
             elem[8] * elem[1] * elem[14] *  elem[7] + elem[0] *  elem[9] * elem[14] *  elem[7] +
            elem[12] * elem[5] *  elem[2] * elem[11] - elem[4] * elem[13] *  elem[2] * elem[11] -
            elem[12] * elem[1] *  elem[6] * elem[11] + elem[0] * elem[13] *  elem[6] * elem[11] +
             elem[4] * elem[1] * elem[14] * elem[11] - elem[0] *  elem[5] * elem[14] * elem[11] -
             elem[8] * elem[5] *  elem[2] * elem[15] + elem[4] *  elem[9] *  elem[2] * elem[15] +
             elem[8] * elem[1] *  elem[6] * elem[15] - elem[0] *  elem[9] *  elem[6] * elem[15] -
             elem[4] * elem[1] * elem[10] * elem[15] + elem[0] *  elem[5] * elem[10] * elem[15];
    }

    //Returns a new inversed matrix from this one. Returns the zero matrix if det ~ 0.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::inversed() const
    {
        T det = determinant();
        if (det < Mathf::Eps)
            return Mat4x4<T>::zero;
        Mat4x4<T> res;

        res.elem[0]  = elem[9]*elem[14]*elem[7] - elem[13]*elem[10]*elem[7] + elem[13]*elem[6]*elem[11] - elem[5]*elem[14]*elem[11] - elem[9]*elem[6]*elem[15] + elem[5]*elem[10]*elem[15];
        res.elem[4]  = elem[12]*elem[10]*elem[7] - elem[8]*elem[14]*elem[7] - elem[12]*elem[6]*elem[11] + elem[4]*elem[14]*elem[11] + elem[8]*elem[6]*elem[15] - elem[4]*elem[10]*elem[15];
        res.elem[8]  = elem[8]*elem[13]*elem[7] - elem[12]*elem[9]*elem[7] + elem[12]*elem[5]*elem[11] - elem[4]*elem[13]*elem[11] - elem[8]*elem[5]*elem[15] + elem[4]*elem[9]*elem[15];
        res.elem[12] = elem[12]*elem[9]*elem[6] - elem[8]*elem[13]*elem[6] - elem[12]*elem[5]*elem[10] + elem[4]*elem[13]*elem[10] + elem[8]*elem[5]*elem[14] - elem[4]*elem[9]*elem[14];
        res.elem[1]  = elem[13]*elem[10]*elem[3] - elem[9]*elem[14]*elem[3] - elem[13]*elem[2]*elem[11] + elem[1]*elem[14]*elem[11] + elem[9]*elem[2]*elem[15] - elem[1]*elem[10]*elem[15];
        res.elem[5]  = elem[8]*elem[14]*elem[3] - elem[12]*elem[10]*elem[3] + elem[12]*elem[2]*elem[11] - elem[0]*elem[14]*elem[11] - elem[8]*elem[2]*elem[15] + elem[0]*elem[10]*elem[15];
        res.elem[9]  = elem[12]*elem[9]*elem[3] - elem[8]*elem[13]*elem[3] - elem[12]*elem[1]*elem[11] + elem[0]*elem[13]*elem[11] + elem[8]*elem[1]*elem[15] - elem[0]*elem[9]*elem[15];
        res.elem[13] = elem[8]*elem[13]*elem[2] - elem[12]*elem[9]*elem[2] + elem[12]*elem[1]*elem[10] - elem[0]*elem[13]*elem[10] - elem[8]*elem[1]*elem[14] + elem[0]*elem[9]*elem[14];
        res.elem[2]  = elem[5]*elem[14]*elem[3] - elem[13]*elem[6]*elem[3] + elem[13]*elem[2]*elem[7] - elem[1]*elem[14]*elem[7] - elem[5]*elem[2]*elem[15] + elem[1]*elem[6]*elem[15];
        res.elem[6]  = elem[12]*elem[6]*elem[3] - elem[4]*elem[14]*elem[3] - elem[12]*elem[2]*elem[7] + elem[0]*elem[14]*elem[7] + elem[4]*elem[2]*elem[15] - elem[0]*elem[6]*elem[15];
        res.elem[10] = elem[4]*elem[13]*elem[3] - elem[12]*elem[5]*elem[3] + elem[12]*elem[1]*elem[7] - elem[0]*elem[13]*elem[7] - elem[4]*elem[1]*elem[15] + elem[0]*elem[5]*elem[15];
        res.elem[14] = elem[12]*elem[5]*elem[2] - elem[4]*elem[13]*elem[2] - elem[12]*elem[1]*elem[6] + elem[0]*elem[13]*elem[6] + elem[4]*elem[1]*elem[14] - elem[0]*elem[5]*elem[14];
        res.elem[3]  = elem[9]*elem[6]*elem[3] - elem[5]*elem[10]*elem[3] - elem[9]*elem[2]*elem[7] + elem[1]*elem[10]*elem[7] + elem[5]*elem[2]*elem[11] - elem[1]*elem[6]*elem[11];
        res.elem[7]  = elem[4]*elem[10]*elem[3] - elem[8]*elem[6]*elem[3] + elem[8]*elem[2]*elem[7] - elem[0]*elem[10]*elem[7] - elem[4]*elem[2]*elem[11] + elem[0]*elem[6]*elem[11];
        res.elem[11] = elem[8]*elem[5]*elem[3] - elem[4]*elem[9]*elem[3] - elem[8]*elem[1]*elem[7] + elem[0]*elem[9]*elem[7] + elem[4]*elem[1]*elem[11] - elem[0]*elem[5]*elem[11];
        res.elem[15] = elem[4]*elem[9]*elem[2] - elem[8]*elem[5]*elem[2] + elem[8]*elem[1]*elem[6] - elem[0]*elem[9]*elem[6] - elem[4]*elem[1]*elem[10] + elem[0]*elem[5]*elem[10];

        res = res * (1 / det);
        return res;
    }

    //ONLY USE ON ROTATION MATRICES!! Returns a new inversed rotation matrix from this rotation matrix. Very fast.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::inversedRotation() const
    {
        return transposed();
    }

    //ONLY USE ON TRANSLATION MATRICES!! Returns a new inversed translation matrix from this translation matrix. Very fast.
    template<typename T> inline
    Mat4x4<T> Mat4x4<T>::inversedTranslation() const
    {
        Mat4x4<T> result = *this;
            result.elem[12] = -result.elem[12];
            result.elem[13] = -result.elem[13];
            result.elem[14] = -result.elem[14];
        return result;
    }

    //ONLY USE ON SCALE MATRICES!! Returns a new inversed scale matrix from this scale matrix. Very fast.
    template<typename T> inline
    Mat4x4<T> Mat4x4<T>::inversedScale() const
    {
        Mat4x4<T> result = *this;
             result.elem[0] = 1 / result.elem[0];
             result.elem[5] = 1 / result.elem[5];
            result.elem[10] = 1 / result.elem[10];
        return result;
    }

    //Returns the i-th column of the matrix.
    template <typename T> inline
    Vec4<T> Mat4x4<T>::getColumn(unsigned int index) const
    {
        return reinterpret_cast<const Vec4<T>*>(this)[index];
    }

    //Returns the i-th row of the matrix.
    template <typename T> inline
    Vec4<T> Mat4x4<T>::getRow(unsigned int index) const
    {
        Vec4<T> result;
        result.x() = elem[index + 0 * 4];
        result.y() = elem[index + 1 * 4];
        result.z() = elem[index + 2 * 4];
        result.w() = elem[index + 3 * 4];
        return result;
    }

    //Transforms a position by this matrix. 4th component of v will be set to 1.
    template <typename T> inline
    Vec3<T> Mat4x4<T>::multiplyPoint(const Vec3<T> & v) const
    {
        Vec4<T> vec4 = Vec4<T>(v.x(), v.y(), v.z(), 1);
        return static_cast<Vec3<T>>(*this * vec4); //Use overloaded Matrix4x4 * Vector4 operator and cast it back to Vector3
    }

    //Transforms a direction by this matrix. 4th component of v will be set to 0.
    template <typename T> inline
    Vec3<T> Mat4x4<T>::multiplyDirection(const Vec3<T> & v) const
    {
        Vec4<T> vec4 = Vec4<T>(v.x(), v.y(), v.z(), 0);
        return static_cast<Vec3<T>>(*this * vec4); //Use overloaded Matrix4x4 * Vector4 operator and cast it back to Vector3
    }


    //Returns an quaternion representing the rotation from this matrix. (Algorithmus from 
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm)
    template <typename T>
    Quaternion<T> Mat4x4<T>::toQuaternion() const
    {
        Quaternion<T> q;

        float trace = elem[0] + elem[5] + elem[10];
        if (trace > 0) {
            float s = 0.5f / sqrtf(trace + 1.0f);
            q.w() = 0.25f / s;
            q.x() = (elem[6] - elem[9]) * s;
            q.y() = (elem[8] - elem[2]) * s;
            q.z() = (elem[1] - elem[4]) * s;
        }
        else {
            if (elem[0] > elem[5] && elem[0] > elem[10]) {
                float s = 2.0f * sqrtf(1.0f + elem[0] - elem[5] - elem[10]);
                q.w() = (elem[6] - elem[9]) / s;
                q.x() = 0.25f * s;
                q.y() = (elem[4] + elem[1]) / s;
                q.z() = (elem[8] + elem[2]) / s;
            }
            else if (elem[5] > elem[10]) {
                float s = 2.0f * sqrtf(1.0f + elem[5] - elem[0] - elem[10]);
                q.w() = (elem[8] - elem[2]) / s;
                q.x() = (elem[4] + elem[1]) / s;
                q.y() = 0.25f * s;
                q.z() = (elem[9] + elem[6]) / s;
            }
            else {
                float s = 2.0f * sqrtf(1.0f + elem[10] - elem[0] - elem[5]);
                q.w() = (elem[1] - elem[4]) / s;
                q.x() = (elem[8] + elem[2]) / s;
                q.y() = (elem[9] + elem[6]) / s;
                q.z() = 0.25f * s;
            }
        }

        return q.normalized();
    }

    //Return an Vec3 representing the translation from this matrix.
    template <typename T> inline
    Vec3<T> Mat4x4<T>::getTranslation() const
    {
        return Vec3<T>(elem[12], elem[13], elem[14]);
    }

    //Return an Vec3 representing the scale from this matrix.
    template <typename T> inline
    Vec3<T> Mat4x4<T>::getScale() const
    {
        float first  = getColumn(0).magnitude();
        float second = getColumn(1).magnitude();
        float third  = getColumn(2).magnitude();

        return Vec3<T>(first, second, third);
    }

    //Return an Quaternion representing the rotation from this matrix.
    //May be incorrect if scale in this matrix is negative.
    template <typename T> inline
    Quaternion<T> Mat4x4<T>::getRotation() const
    {
        return getRotation(getScale());
    }
    
    //Return an Quaternion representing the rotation from this matrix.
    //Give the scale (which is needed for the calculation) separately to this func.
    //May be incorrect if scale in this matrix is negative.
    //Algorithm from: http://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati
    template <typename T> inline
    Quaternion<T> Mat4x4<T>::getRotation(const Vec3<T>& scale) const
    {
        if(scale.x() == 0 || scale.y() == 0 || scale.z() == 0)
            return Quaternion<T>::identity;

        Mat4f rotMat;
        rotMat[0] = (*this)[0] / scale.x();
        rotMat[1] = (*this)[1] / scale.y();
        rotMat[2] = (*this)[2] / scale.z();
        rotMat[3] = Vec4f(0, 0, 0, 1);

        return rotMat.toQuaternion();
    }

    //---------------------------------------------------------------------------
    //  Nonconst member functions
    //---------------------------------------------------------------------------

    //Transpose this matrix and returns a reference.
    template <typename T> inline
    Mat4x4<T>& Mat4x4<T>::transpose()
    {
        *this = this->transposed();
        return *this;
    }

    //Inverse this matrix and returns a reference.
    template <typename T> inline
    Mat4x4<T>& Mat4x4<T>::inverse()
    {
        *this = this->inversed();
        return *this;
    }

    //Set a specific element
    template <typename T> inline
    void Mat4x4<T>::set(unsigned int row, unsigned int col, T val)
    {
        elem[row + 4 * col] = val;
    }

    //---------------------------------------------------------------------------
    //  Static member functions
    //---------------------------------------------------------------------------

    //Creates and returns a new uniformed scale matrix.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::scale(T s)
    {
        return Mat4x4<T>(s, 0, 0, 0,
                         0, s, 0, 0,
                         0, 0, s, 0,
                         0, 0, 0, 1);
    }

    //Creates a new nonuniformed scale matrix.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::scale(T s1, T s2, T s3)
    {
        return Mat4x4<T>(s1,  0,  0, 0,
                          0, s2,  0, 0,
                          0,  0, s3, 0,
                          0,  0,  0, 1);
    }

    //Creates a new nonuniformed scale matrix.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::scale(const Vec3<T> & s)
    {
        return Mat4x4<T>(s.x(),     0,     0, 0,
                             0, s.y(),     0, 0,
                             0,     0, s.z(), 0,
                             0,     0,     0, 1);
    }

    //Creates and returns a new translation matrix.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::translation(T x, T y, T z)
    {
        return Mat4x4<T>(1, 0, 0, x,
                         0, 1, 0, y,
                         0, 0, 1, z,
                         0, 0, 0, 1);
    }

    //Creates and returns a new translation matrix.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::translation(const Vec3<T> & trans)
    {
        return Mat4x4<T>(1, 0, 0, trans.x(),
                         0, 1, 0, trans.y(),
                         0, 0, 1, trans.z(),
                         0, 0, 0,       1);
    }

    //Creates a translation, rotation and scaling matrix.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::trs(const Vec3<T> & trans, const Quaternion<T> & q, const Vec3<T> & scale)
    {
        Mat4x4<T> result(1 - 2 * (q.y()*q.y() + q.z()*q.z()),     2 * (q.x()*q.y() - q.w()*q.z()),     2 * (q.x()*q.z() + q.w()*q.y()), trans.x(),
                             2 * (q.x()*q.y() + q.w()*q.z()), 1 - 2 * (q.x()*q.x() + q.z()*q.z()),     2 * (q.y()*q.z() - q.w()*q.x()), trans.y(),
                             2 * (q.x()*q.z() - q.w()*q.y()),     2 * (q.y()*q.z() + q.w()*q.x()), 1 - 2 * (q.x()*q.x() + q.y()*q.y()), trans.z(),
                                                           0,                                   0,                                   0,        1);

        result *= Mat4x4<T>::scale(scale.x(), scale.y(), scale.z());

        return result;
    }

    //Creates a rotation matrix from a forward and up vector. Calculating the right vector.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::initRotation(const Vec3<T> & forward, const Vec3<T> & up)
    {
        Vec3<T> f = forward.normalized();

        Vec3<T> r = up.normalized();
        r = r.cross(f);

        Vec3<T> u = f.cross(r);

        return Mat4x4<T>::initRotation(f, u, r);
    }

    //Creates a rotation matrix from a forward, up and right vector
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::initRotation(const Vec3<T> & forward, const Vec3<T> & up, const Vec3<T> & right)
    {
        return Mat4x4<T>(  right.x(),   right.y(),   right.z(), 0,
                              up.x(),      up.y(),      up.z(), 0,
                         forward.x(), forward.y(), forward.z(), 0,
                                   0,           0,           0, 1);
    }

    //Creates a new matrix, which rotates RADIANS about the X-axis.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::rotationX(float radians)
    {
        float sinAngle = sin(radians);
        float cosAngle = cos(radians);

        return Mat4x4<T>(1,        0,         0, 0,
                         0, cosAngle, -sinAngle, 0,
                         0, sinAngle,  cosAngle, 0,
                         0,        0,         0, 1);
    }

    //Creates a new matrix, which rotates RADIANS about the Y-axis.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::rotationY(float radians)
    {
        float sinAngle = sin(radians);
        float cosAngle = cos(radians);

        return Mat4x4<T>( cosAngle, 0, sinAngle, 0,
                                 0, 1,        0, 0,
                         -sinAngle, 0, cosAngle, 0,
                                 0, 0,        0, 1);
    }

    //Creates a new matrix, which rotates RADIANS about the Z-axis. 
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::rotationZ(float radians)
    {
        float sinAngle = sin(radians);
        float cosAngle = cos(radians);

        return Mat4x4<T>(cosAngle, -sinAngle, 0, 0,
                         sinAngle,  cosAngle, 0, 0,
                                0,         0, 1, 0,
                                0,         0, 0, 1);
    }

    //Creates a new matrix, which rotates RADIANS about any arbitrary axis, given by a vec3-argument.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::rotationArbitrary(const Vec3<T> & ax, float radians)
    {
        Vec3<T> axis = ax.normalized();
        
        float sinAngle = sin(radians);
        float cosAngle = cos(radians);

        float oneMinusCosAngle = 1 - cosAngle;

        return Mat4x4<T>(           axis.x() * axis.x() * oneMinusCosAngle + cosAngle, axis.x() * axis.y() * oneMinusCosAngle - axis.z() * sinAngle, axis.x() * axis.z() * oneMinusCosAngle + axis.y() * sinAngle, 0,
                         axis.y() * axis.x() * oneMinusCosAngle + axis.z() * sinAngle,            axis.y() * axis.y() * oneMinusCosAngle + cosAngle, axis.y() * axis.z() * oneMinusCosAngle - axis.x() * sinAngle, 0,
                         axis.z() * axis.x() * oneMinusCosAngle - axis.y() * sinAngle, axis.z() * axis.y() * oneMinusCosAngle + axis.x() * sinAngle,            axis.z() * axis.z() * oneMinusCosAngle + cosAngle, 0,
                                                                                    0,                                                            0,                                                            0, 1);
    }

    //Creates a new matrix, which looks at the target. Algorithmus from http://stackoverflow.com/questions/349050/calculating-a-lookat-matrix
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::lookAt(const Vec3<T> &pos, const Vec3<T> &target, const Vec3<T> &up)
    {
        Vec3<T> zAxis = (pos - target).normalized();
        Vec3<T> xAxis = (up.cross(zAxis)).normalized();
        Vec3<T> yAxis = zAxis.cross(xAxis);
        
        return Mat4x4<T>(xAxis.x(), xAxis.y(), xAxis.z(), -xAxis.dot(pos),
                         yAxis.x(), yAxis.y(), yAxis.z(), -yAxis.dot(pos),
                         zAxis.x(), zAxis.y(), zAxis.z(), -zAxis.dot(pos),
                                 0,         0,         0,              1);
    }

    //Creates a new view matrix
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::view(const Vec3<T> &cam, const Vec3<T> &forward, const Vec3<T> &up)
    {
        Vec3<T> zAxis = -forward.normalized();
        Vec3<T> xAxis = (up.cross(zAxis)).normalized();
        Vec3<T> yAxis = zAxis.cross(xAxis);

        return Mat4x4<T>(xAxis.x(), xAxis.y(), xAxis.z(), -xAxis.dot(cam),
                         yAxis.x(), yAxis.y(), yAxis.z(), -yAxis.dot(cam),
                         zAxis.x(), zAxis.y(), zAxis.z(), -zAxis.dot(cam),
                                 0,         0,         0,              1);
    }

    //Creates and returns an orthographic projection matrix.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::ortho(T left, T right, T bottom, T top, T _near, T _far)
    {
        return Mat4x4<T>(2 / (right - left),                   0,                   0, -((right + left) / (right - left)),
                                           0, 2 / (top - bottom),                   0, -((top + bottom) / (top - bottom)),
                                           0,                  0, -2 / (_far - _near), -((_far + _near) / (_far - _near)),
                                           0,                  0,                   0,                                 1);
    }

    //Creates and returns an orthographic projection matrix without a scaling.
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::ortho2(T left, T right, T bottom, T top, T _near, T _far)
    {
        return Mat4x4<T>(2 / (right - left),                  0,                   0, 0,
                                          0, 2 / (top - bottom),                   0, 0,
                                          0,                  0, 2 / (_far - _near),  0,
                                          0,                  0,                   0, 1);
    }

    //Creates an perspective projection matrix. GLM Version
    template <typename T> inline
    Mat4x4<T> Mat4x4<T>::perspective(T fov, T ar, T _near, T _far)
    {
        float halfTanFOV = tan(0.5f * fov);
        float zRange = _far - _near;
        return Mat4x4<T>(1.0f / (halfTanFOV * ar),                 0,                        0,                            0,
                                                0, 1.0f / halfTanFOV,                        0,                            0,
                                                0,                 0, -(_far + _near) / zRange, -(2 * _far * _near) / zRange,
                                                0,                 0,                       -1,                            0);
    }


}
#endif