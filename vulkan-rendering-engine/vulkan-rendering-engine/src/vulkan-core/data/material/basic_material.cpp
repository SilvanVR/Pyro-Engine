#include "basic_material.h"

#include "vulkan-core/resource_manager/texture_manager.h"
#include "vulkan-core/resource_manager/shader_manager.h"

namespace Pyro
{

    // Create a material, using the phong-shader.
    // The given texture is the diffuse-texture.
    // It sets default values for phong-shading (specular-intensity etc.).
    BasicMaterial::BasicMaterial(const Texture* texture, const Color& color, const std::string& name, Object::EType type)
        : Material(ShaderManager::get("GBuffer"), name, type)
    {
        // Given Texture in the constructor is always the diffuse texture
        if (texture == nullptr)
            setMatDiffuseTexture(TextureManager::get(TEX_DEFAULT));
        else
            setMatDiffuseTexture(texture);

        // Some default-values for phong-shading
        setMatSpecularIntensity(2);
        setMatUVScale(1.0f);
        setMatColor(color);

        // Normal mapping
        setMatNormalMap(TextureManager::get(TEX_DEFAULT_NORMAL));

        // Parallax Displacement mapping
        setMatDisplacementMap(TextureManager::get(TEX_DEFAULT_DISPLACEMENT));
        setMatDisplacementScale(0.0f);
        setMatDisplacementBias(0.0f);
    }


    void BasicMaterial::setMatDiffuseTexture(const Texture* diffuse)
    {
        MappedValues::setTexture(SHADER_DIFFUSE_MAP_NAME, diffuse);
    }

    void BasicMaterial::setMatNormalMap(const Texture* normalMap)
    {
        MappedValues::setTexture(SHADER_NORMAL_MAP_NAME, normalMap);
    }

    void BasicMaterial::setMatDisplacementMap(const Texture* dispMap)
    {
        MappedValues::setTexture(SHADER_DISPLACEMENT_MAP_NAME, dispMap);
    }

    void BasicMaterial::setMatColor(const Color& color)
    {
        MappedValues::setColor(SHADER_COLOR_NAME, color);
    }

    void BasicMaterial::setMatSpecularIntensity(float specularIntensity)
    {
        MappedValues::setFloat(SHADER_SPECULAR_INTENSITY_NAME, specularIntensity);
    }

    void BasicMaterial::setMatUVScale(float uvScale)
    {
        MappedValues::setFloat(SHADER_UV_SCALE_NAME, uvScale);
    }

    void BasicMaterial::setMatDisplacementScale(float scale)
    {
        MappedValues::setFloat(SHADER_DISPLACEMENT_SCALE_NAME, scale);
    }

    void BasicMaterial::setMatDisplacementBias(float bias)
    {
        MappedValues::setFloat(SHADER_DISPLACEMENT_BIAS_NAME, bias);
    }



}