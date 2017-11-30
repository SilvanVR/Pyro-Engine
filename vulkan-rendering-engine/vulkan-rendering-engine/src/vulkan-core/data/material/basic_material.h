#ifndef BASIC_MATERIAL_H_
#define BASIC_MATERIAL_H_

#include "material.h"


namespace Pyro
{
    // Defines for matching the shader-names exactly
    #define SHADER_DIFFUSE_MAP_NAME         "diffuse"
    #define SHADER_NORMAL_MAP_NAME          "normalMap"
    #define SHADER_DISPLACEMENT_MAP_NAME    "dispMap"
    #define SHADER_COLOR_NAME               "color"
    #define SHADER_SPECULAR_INTENSITY_NAME  "specularIntensity"
    #define SHADER_UV_SCALE_NAME            "uvScale"
    #define SHADER_DISPLACEMENT_SCALE_NAME  "dispScale"
    #define SHADER_DISPLACEMENT_BIAS_NAME   "dispBias"


    class BasicMaterial : public Material
    {

    public:
        // Create a material using the standard shader.
        // The given texture is the diffuse-texture.
        // It sets default values for all necessary fields (specular-intensity etc.).
        BasicMaterial(const Texture* diffuse, const Color& color = Color::WHITE, const std::string& name = "", Object::EType type = Object::LOCAL);
        ~BasicMaterial() {}

        // Basic functions for settings all shader relevant fields for a material of this kind
        void setMatDiffuseTexture(const Texture* diffuse);
        void setMatNormalMap(const Texture* normalMap);
        void setMatDisplacementMap(const Texture* dispMap);
        void setMatColor(const Color& color);
        void setMatSpecularIntensity(float specularIntensity);
        void setMatUVScale(float uvScale);
        void setMatDisplacementScale(float scale);
        void setMatDisplacementBias(float bias);
    };

}



#endif // !BASIC_MATERIAL_H_

