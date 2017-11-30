/*
*  Matrix3x3 - Class header file.  
*
*  Date:    24.03.2016
*  Creator: Silvan Hau
*/
#ifndef _MAT3_H_
#define _MAT3_H_

#include "util.h" /* Mathf::PI, Mathf::clamp, ...                                                         */
#include "Vector3.h"               /* Rows interpreted as Vector-3 with [] operator --- Vec3<T>& operator[] (unsigned int) */
//#include "Quaternion.h"            /* Convert to quaternion                                                                */
//#include "Matrix4x4.h"             /* Convert to Matrix4x4                                                                 */

 //---------------------------------------------------------------------------
 //  Forward Declarations
 //---------------------------------------------------------------------------
template <typename T> class Vec3;
template <typename T> class Quaternion;
template <typename T> class Mat4x4;

namespace math
{

//---------------------------------------------------------------------------
//  3x3 - Matrix Class
//---------------------------------------------------------------------------
    template <typename T>
    class Mat3x3
    {
        private:
        // [row + col * 3]
        T elem[9];

        public:
        //Constructors
        Mat3x3(T value = 0)
        {
            elem[0 + 0 * 3] = elem[0 + 1 * 3] = elem[0 + 2 * 3] = value;
            elem[1 + 0 * 3] = elem[1 + 1 * 3] = elem[1 + 2 * 3] = value;
            elem[2 + 0 * 3] = elem[2 + 1 * 3] = elem[2 + 2 * 3] = value;
        }
        
        Mat3x3(
            T i11, T i12, T i13,
            T i21, T i22, T i23,
            T i31, T i32, T i33
        )
        {
            elem[0 + 0 * 3] = i11; elem[0 + 1 * 3] = i12; elem[0 + 2 * 3] = i13;
            elem[1 + 0 * 3] = i21; elem[1 + 1 * 3] = i22; elem[1 + 2 * 3] = i23;
            elem[2 + 0 * 3] = i31; elem[2 + 1 * 3] = i32; elem[2 + 2 * 3] = i33;
        }
        
        //Conversion Functions
        explicit operator Mat4x4<T>() const { return Mat4x4<T>(elem[0], elem[3], elem[6], 0,
                                                               elem[1], elem[4], elem[7], 0,
                                                               elem[2], elem[5], elem[8], 0,
                                                                     0,       0,       0, 1); }
        
        //Operators
        Mat3x3<T>       operator+       (const Mat3x3<T> &)     const;
        Mat3x3<T>       operator-       (const Mat3x3<T> &)     const;
        Mat3x3<T>       operator*       (const Mat3x3<T> &)     const;
        Mat3x3<T>       operator*       (T)                     const;
        Mat3x3<T>       operator/       (T)                     const;
        Mat3x3<T>       operator-       ()                      const;
        bool            operator==      (const Mat3x3<T> &)     const;
        bool            operator!=      (const Mat3x3<T> &)     const;
        const Vec3<T>&  operator[]      (unsigned int)          const;
        Vec3<T>&        operator[]      (unsigned int);
        void            operator+=      (const Mat3x3<T> &);
        void            operator-=      (const Mat3x3<T> &);
        void            operator*=      (const Mat3x3<T> &);
        
        //Const member functions
        Mat3x3<T>       multiply        (const Mat3x3<T> &m)    const { return *this * m; } //Standard matrix-multiplication (uses overloaded "*" operator)
        Mat3x3<T>       transposed      ()                      const;                      //Returns a new transposed matrix from this one.
        T               determinant     ()                      const;                      //Returns the determinant for this matrix.
        Mat3x3<T>       inversed        ()                      const;                      //Returns a new inversed matrix from this one.
        Mat3x3<T>       inversedRotation()                      const;                      //Returns a new inversed rotation matrix from this one.
        Mat3x3<T>       inversedScale   ()                      const;                      //Returns a new inversed scale matrix from this one.
        Vec3<T>         getColumn       (unsigned int)          const;                      //Returns the i-th colum of the matrix.
        Vec3<T>         getRow          (unsigned int)          const;                      //Returns the i-th row of the matrix.
        Quaternion<T>   toQuaternion    ()                      const;                      //Returns an quaternion representing the rotation from this matrix.
        
        //Nonconst member functions
        Mat3x3<T>&      transpose       ();                                                 //Transpose this matrix and returns it as a reference.
        Mat3x3<T>&      inverse         ();                                                 //Inverse this matrix and returns it as a reference.
        
        //Static members
        static const Mat3x3<T> identity;                                                    //The identity matrix
        static const Mat3x3<T> zero;                                                        //The zero matrix
        
        //Static member functions
        static Mat3x3<T> scale            (T);                                              //Creates a new uniformed scale matrix.
        static Mat3x3<T> rotationX        (float);                                          //Creates a new matrix, which rotates about the X-axis.
        static Mat3x3<T> rotationY        (float);                                          //Creates a new matrix, which rotates about the Y-axis.
        static Mat3x3<T> rotationZ        (float);                                          //Creates a new matrix, which rotates about the Z-axis.
        static Mat3x3<T> rotationArbitrary(const Vec3<T> &, float);                         //Creates a new matrix, which rotates about any arbitrary axis, given by a vec3-argument and an angle.
        
    };
    
    //---------------------------------------------------------------------------
    //  Static Member Initialization
    //---------------------------------------------------------------------------
    template <typename T> const Mat3x3<T> Mat3x3<T>::identity = Mat3x3<T>(1, 0, 0,
                                                                          0, 1, 0,
                                                                          0, 0, 1);
    
    template <typename T> const Mat3x3<T> Mat3x3<T>::zero     = Mat3x3<T>(0, 0, 0,
                                                                          0, 0, 0,
                                                                          0, 0, 0);
    
    //---------------------------------------------------------------------------
    //  Operator Overloading
    //---------------------------------------------------------------------------
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::operator+(const Mat3x3<T> & m) const
    {
        return Mat3x3<T>(elem[0] + m.elem[0], elem[3] + m.elem[3],  elem[6] +  m.elem[6],
                         elem[1] + m.elem[1], elem[4] + m.elem[4],  elem[7] +  m.elem[7],
                         elem[2] + m.elem[2], elem[5] + m.elem[5],  elem[8] +  m.elem[8]);
    }
    
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::operator-(const Mat3x3<T> & m) const
    {
        return Mat3x3<T>(elem[0] - m.elem[0], elem[3] - m.elem[3],  elem[6] -  m.elem[6],
                         elem[1] - m.elem[1], elem[4] - m.elem[4],  elem[7] -  m.elem[7],
                         elem[2] - m.elem[2], elem[5] - m.elem[5],  elem[8] -  m.elem[8]);
    }
    
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::operator*(const Mat3x3<T> & m) const
    {
        return Mat3x3<T>(elem[0] * m.elem[0] + elem[3] * m.elem[1] + elem[6] * m.elem[2], elem[0] * m.elem[3] + elem[3] * m.elem[4] + elem[6] * m.elem[5], elem[0] * m.elem[6] + elem[3] * m.elem[7] + elem[6] * m.elem[8],
                         elem[1] * m.elem[0] + elem[4] * m.elem[1] + elem[7] * m.elem[2], elem[1] * m.elem[3] + elem[4] * m.elem[4] + elem[7] * m.elem[5], elem[1] * m.elem[6] + elem[4] * m.elem[7] + elem[7] * m.elem[8],
                         elem[2] * m.elem[0] + elem[5] * m.elem[1] + elem[8] * m.elem[2], elem[2] * m.elem[3] + elem[5] * m.elem[4] + elem[8] * m.elem[5], elem[2] * m.elem[6] + elem[5] * m.elem[7] + elem[8] * m.elem[8]);
    }
    
    // "*" operator for mat * scalar (member func) AND scalar * mat (nonmember func)
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::operator*(T s) const
    {
        return Mat3x3<T>(elem[0] * s, elem[3] * s, elem[6] * s,
                         elem[1] * s, elem[4] * s, elem[7] * s,
                         elem[2] * s, elem[5] * s, elem[8] * s);
    }
    template <typename T, typename T2> inline
    Mat3x3<T> operator*(T2 s, const Mat3x3<T> & m)
    {
        return m * s;
    }
    
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::operator/(T s) const
    {
        return Mat3x3<T>(elem[0] / s, elem[3] / s, elem[6] / s,
                         elem[1] / s, elem[4] / s, elem[7] / s,
                         elem[2] / s, elem[5] / s, elem[8] / s);
    }
    
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::operator-() const
    {
        return Mat3x3<T>(-elem[0], -elem[3], -elem[6],
                         -elem[1], -elem[4], -elem[7],
                         -elem[2], -elem[5], -elem[8]);
    }
    
    template <typename T> inline
    bool Mat3x3<T>::operator== (const Mat3x3<T> & m) const
    {
        return elem[0] == m.elem[0] && elem[3] == m.elem[3] && elem[6] == m.elem[6] &&
               elem[1] == m.elem[1] && elem[4] == m.elem[4] && elem[7] == m.elem[7] &&
               elem[2] == m.elem[2] && elem[5] == m.elem[5] && elem[8] == m.elem[8];
    }
    
    template <typename T> inline
    bool Mat3x3<T>::operator!=(const Mat3x3<T> & m) const
    {
        return !(*this == m);
    }
    
    template <typename T> inline
    void Mat3x3<T>::operator+= (const Mat3x3<T> & m)
    {
        elem[0] += m.elem[0]; elem[3] += m.elem[3]; elem[6] += m.elem[6];
        elem[1] += m.elem[1]; elem[4] += m.elem[4]; elem[7] += m.elem[7];
        elem[2] += m.elem[2]; elem[5] += m.elem[5]; elem[8] += m.elem[8];
    }
    
    template <typename T> inline
    void Mat3x3<T>::operator-= (const Mat3x3<T> & m)
    {
        elem[0] -= m.elem[0]; elem[3] -= m.elem[3]; elem[6] -= m.elem[6];
        elem[1] -= m.elem[1]; elem[4] -= m.elem[4]; elem[7] -= m.elem[7];
        elem[2] -= m.elem[2]; elem[5] -= m.elem[5]; elem[8] -= m.elem[8];
    }
    
    template <typename T> inline
    void Mat3x3<T>::operator*= (const Mat3x3<T> & m)
    {
        *this = *this * m;
    }
    
    template <typename T> inline
    const Vec3<T>& Mat3x3<T>::operator[](unsigned int index) const
    {
        return reinterpret_cast<const Vec3<T>*>(this)[index];
    }
    
    template <typename T> inline
    Vec3<T>& Mat3x3<T>::operator[](unsigned int index)
    {
        return reinterpret_cast<Vec3<T>*>(this)[index];
    }
    
    //---------------------------------------------------------------------------
    //  Const member functions
    //---------------------------------------------------------------------------
    
    //Returns a new transposed matrix from this one.
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::transposed() const
    {
        return Mat3x3<T>(elem[0], elem[1], elem[2],
                         elem[3], elem[4], elem[5],
                         elem[6], elem[7], elem[8]);
    }
    
    //Returns the determinant for this matrix.
    template <typename T> inline
    T Mat3x3<T>::determinant() const
    {
        return elem[0]*elem[4]*elem[8] + elem[3]*elem[7]*elem[2] + elem[6]*elem[1]*elem[5] - elem[0]*elem[7]*elem[5] - elem[3]*elem[1]*elem[8] - elem[6]*elem[4]*elem[2];
    }
    
    //Returns a new inversed matrix from this one. Returns the zero matrix if det = 0.
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::inversed()	const
    {
        T det = (*this).determinant();
        if (det == 0)
            return Mat3x3<T>::zero;
        Mat3x3<T> res;
        res.elem[0] = (elem[4]*elem[8] - elem[7]*elem[5]) / det;
        res.elem[3] = (elem[6]*elem[5] - elem[3]*elem[8]) / det;
        res.elem[6] = (elem[3]*elem[7] - elem[6]*elem[4]) / det;
        res.elem[1] = (elem[7]*elem[2] - elem[1]*elem[8]) / det;
        res.elem[4] = (elem[0]*elem[8] - elem[6]*elem[2]) / det;
        res.elem[7] = (elem[6]*elem[1] - elem[0]*elem[7]) / det;
        res.elem[2] = (elem[1]*elem[5] - elem[4]*elem[2]) / det;
        res.elem[5] = (elem[3]*elem[2] - elem[0]*elem[5]) / det;
        res.elem[8] = (elem[0]*elem[4] - elem[3]*elem[1]) / det;
        return res;
    }
    
    //ONLY USE ON ROTATION MATRICES!! Returns a new inversed rotation matrix from this rotation matrix. Very fast.
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::inversedRotation()	const
    {
        return (*this).transposed();
    }
    
    //ONLY USE ON SCALE MATRICES!! Returns a new inversed scale matrix from this scale matrix. Very fast.
    template<typename T> inline
    Mat3x3<T> Mat3x3<T>::inversedScale() const
    {
        Mat3x3<T> result = *this;
            result.elem[0] = 1 / result.elem[0];
            result.elem[4] = 1 / result.elem[4];
            result.elem[8] = 1 / result.elem[8];
        return result;
    }
    
    //Returns the i-th column of the matrix.
    template <typename T> inline
    Vec3<T>	Mat3x3<T>::getColumn(unsigned int index) const
    {
        return reinterpret_cast<const Vec3<T>*>(this)[index];
    }
    
    //Returns the i-th row of the matrix.
    template <typename T> inline
    Vec3<T>	Mat3x3<T>::getRow(unsigned int index) const
    {
        Vec3<T> result;
        result.x() = elem[index + 0 * 3];
        result.y() = elem[index + 1 * 3];
        result.z() = elem[index + 2 * 3];
        return result;
    }
    
    
    //---------------------------------------------------------------------------
    //  Nonconst member functions
    //---------------------------------------------------------------------------
    
    //Transpose this matrix and returns a reference.
    template <typename T> inline
    Mat3x3<T>& Mat3x3<T>::transpose()
    {
        *this = this->transposed();
        return *this;
    }
    
    //Inverse this matrix and returns a reference.
    template <typename T> inline
    Mat3x3<T>& Mat3x3<T>::inverse()
    {
        *this = this->inversed();
        return *this;
    }
    
    //Returns an quaternion representing the rotation from this matrix. (Algorithmus from 
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm)
    template <typename T>
    Quaternion<T> Mat3x3<T>::toQuaternion() const
    {
        Quaternion<T> q;
        
        float trace = elem[0] + elem[4] + elem[8];
        if (trace > 0) {
            float s = 0.5f / sqrtf(trace + 1.0f);
            q.w() = 0.25f / s;
            q.x() = (elem[5] - elem[7]) * s;
            q.y() = (elem[6] - elem[2]) * s;
            q.z() = (elem[1] - elem[3]) * s;
        }
        else {
            if (elem[0] > elem[4] && elem[0] > elem[8]) {
                float s = 2.0f * sqrtf(1.0f + elem[0] - elem[4] - elem[8]);
                q.w() = (elem[5] - elem[7]) / s;
                q.x() = 0.25f * s;
                q.y() = (elem[3] + elem[1]) / s;
                q.z() = (elem[6] + elem[2]) / s;
            }
            else if (elem[4] > elem[8]) {
                float s = 2.0f * sqrtf(1.0f + elem[4] - elem[0] - elem[8]);
                q.w() = (elem[6] - elem[2]) / s;
                q.x() = (elem[3] + elem[1]) / s;
                q.y() = 0.25f * s;
                q.z() = (elem[7] + elem[5]) / s;
            }
            else {
                float s = 2.0f * sqrtf(1.0f + elem[8] - elem[0] - elem[4]);
                q.w() = (elem[1] - elem[3]) / s;
                q.x() = (elem[6] + elem[2]) / s;
                q.y() = (elem[7] + elem[5]) / s;
                q.z() = 0.25f * s;
            }
        }
        return q;
    }
    
    //---------------------------------------------------------------------------
    //  Static member functions
    //---------------------------------------------------------------------------

    //Creates and returns a new uniformed scale matrix.
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::scale(T s)
    {
        return Mat3x3<T>(s, 0, 0,
                         0, s, 0,
                         0, 0, s);
    }
    
    //Creates a new matrix, which rotates RADIANS about the X-axis.  
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::rotationX(float radians)
    {
        float sinAngle = sin(radians);
        float cosAngle = cos(radians);
        
        return Mat3x3<T>(1,        0,         0,
                         0, cosAngle, -sinAngle,
                         0, sinAngle,  cosAngle);
    }
    
    //Creates a new matrix, which rotates RADIANS about the Y-axis. 
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::rotationY(float radians)
    {
        float sinAngle = sin(radians);
        float cosAngle = cos(radians);
        
        return Mat3x3<T>(cosAngle, 0, sinAngle,
                                0, 1,        0,
                        -sinAngle, 0, cosAngle);
    }
    
    //Creates a new matrix, which rotates RADIANS about the Z-axis.
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::rotationZ(float radians)
    {
        float sinAngle = sin(radians);
        float cosAngle = cos(radians);
        
        return Mat3x3<T>(cosAngle, -sinAngle, 0,
                         sinAngle,  cosAngle, 0,
                                0,         0, 1);
    }
    
    //Creates a new matrix, which rotates RADIANS about any arbitrary axis, given by a vec3-argument.
    template <typename T> inline
    Mat3x3<T> Mat3x3<T>::rotationArbitrary(const Vec3<T> & ax, float radians)
    {
        Vec3<T> axis = ax.normalized();
        
        float sinAngle = sin(radians);
        float cosAngle = cos(radians);
        
        float oneMinusCosAngle = 1 - cosAngle;
        
        return Mat3x3<T>(
                          axis.x() * axis.x() * oneMinusCosAngle + cosAngle, axis.x() * axis.y() * oneMinusCosAngle - axis.z() * sinAngle, axis.x() * axis.z() * oneMinusCosAngle + axis.y() * sinAngle,
               axis.y() * axis.x() * oneMinusCosAngle + axis.z() * sinAngle,            axis.y() * axis.y() * oneMinusCosAngle + cosAngle, axis.y() * axis.z() * oneMinusCosAngle - axis.x() * sinAngle,
               axis.z() * axis.x() * oneMinusCosAngle - axis.y() * sinAngle, axis.z() * axis.y() * oneMinusCosAngle + axis.x() * sinAngle,            axis.z() * axis.z() * oneMinusCosAngle + cosAngle
               );
    }

}
#endif