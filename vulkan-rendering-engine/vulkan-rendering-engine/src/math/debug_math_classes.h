#pragma once

#include <iostream> /* cout          */  //SH: wird später entfernt wenn ich die debug - methoden bei den klassen rausnehme
#include <iomanip>  /* setw(width)   */

#include "math_interface.h"

namespace math
{
	template <typename T>
	std::ostream & operator<<(std::ostream & os, const Vec2<T> & vec)
	{
		os << "(" << vec.x() << "," << vec.y() << ")";
		return os;
	}

	template <typename T>
	std::ostream & operator<<(std::ostream & os, const Vec3<T> & vec)
	{
		os << "(" << vec.x() << "," << vec.y() << "," << vec.z() << ")";
		return os;
	}

	template <typename T>
	std::ostream & operator<<(std::ostream & os, const Vec4<T> & vec)
	{
		os << "(" << vec.x() << "," << vec.y() << "," << vec.z() << "," << vec.w() << ")";
		return os;
	}

	template <typename T>
	std::ostream & operator<<(std::ostream & os, const Mat3x3<T> & m)
	{
        using namespace std;
        int width = 10;
        for (int col = 0; col < 3; col++)
            os << "|" << setw(width) << m[0][col] << "," << setw(width) << m[1][col] << "," << setw(width) << m[2][col] << " |" << endl;
        return os;
	}

	template <typename T>
	std::ostream & operator<<(std::ostream & os, const Mat4x4<T> & m)
	{
		using namespace std;
		int width = 10;
		for (int col = 0; col < 4; col++)
				os << "|" << setw(width) << m[0][col] << "," << setw(width) << m[1][col] << "," << setw(width) << m[2][col] << "," << setw(width) << m[3][col] << " |" << endl;
		return os;
	}

	template <typename T>
	std::ostream & operator<<(std::ostream & os, const Quaternion<T> & quat)
	{
		os << "(" << quat.x() << "," << quat.y() << "," << quat.z() << "," << quat.w() << ")";
		return os;
	}
}