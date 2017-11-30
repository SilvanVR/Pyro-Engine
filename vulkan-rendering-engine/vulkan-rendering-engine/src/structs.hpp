/*
*  Structs - header file.
*  Contains all custom structs and enums used in this program.
*
*  Date:    23.04.2016
*  Creator: Silvan Hau
*/

#ifndef STRUCTS_H_
#define STRUCTS_H_

#include <vector>
#include <functional>
#include "platform.hpp"
#include "math/math_interface.h"

namespace Pyro
{

    #define INVALID_CALLBACK_ID 0

    // Vertex layout
    struct Vertex
    {
        Vec3f position;
        Vec2f uv;
        Vec3f normal;
        Vec3f tangent;
        Vec3f biTangent;
    };

    // Defines all possible shader types for the Shader-Class
    enum class ShaderStage
    {
        Vertex,
        Geometry,
        Tessellation,
        Fragment
    };

    // Self-Explaining i guess
    enum class DataType
    {
        Boolean,
        Char,
        Float,
        Int,
        Double,
        Vec2,
        Vec3,
        Vec4,
        Mat4,
        Color,
        Texture2D,
        Texture3D,
        TextureCubemap,
        Struct,

        UNKNOWN
    };

    // Contains information about what has been rendered. 
    // Used as a return result from the engine.
    struct ImageData
    {
        std::vector<unsigned char> pixels;
        Vec2ui                     resolution;
        uint32_t                   bytesPerPixel;
    };

    using CallbackID = unsigned char;
    struct CallbackInfo
    {
        CallbackID id;
        std::function<void()> callback;
    };

}

#endif // !STRUCTS_H_