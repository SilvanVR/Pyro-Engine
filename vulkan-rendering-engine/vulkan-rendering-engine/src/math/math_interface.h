/*
*  Math - Interface. Include this to use all MATH files.
*
*  Date:    29.03.2016
*  Creator: Silvan Hau
*
*  Version 1.0.1
*/
#ifndef _MATH_H_
#define _MATH_H_

#include "util.h" //Includes <math.h>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Random.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"
#include "Quaternion.h"
#include "Point.h"
#include "Rectangle.h"
#include "debug_math_classes.h"

using namespace math;

//Explicit instantiation
template class math::Vec2<float>;
template class math::Vec3<float>;
template class math::Vec4<float>;
template class math::Quaternion<float>;
template class math::Mat3x3<float>;
template class math::Mat4x4<float>;
template class math::Rectangle<float>;

//same as typedef
namespace math
{
    using Vec2i   = math::Vec2<int>;
    using Vec2ui  = math::Vec2<unsigned int>;
    using Vec2f   = math::Vec2<float>;
    using Vec3f   = math::Vec3<float>;
    using Vec4f   = math::Vec4<float>;
    using Quatf   = math::Quaternion<float>;
    using Mat3f   = math::Mat3x3<float>;
    using Mat4f   = math::Mat4x4<float>;
    using Point3f = math::Point<float>;
    using Rectf   = math::Rectangle<float>;
}

#endif //_MATH_H_
