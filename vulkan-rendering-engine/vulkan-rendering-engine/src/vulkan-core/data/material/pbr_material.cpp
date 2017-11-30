#include "pbr_material.h"

#include "vulkan-core/resource_manager/resource_manager.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------

    PBRMaterial::PBRMaterial(const PBRMaterialParams& params)
        : Material(SHADER("GBuffer"), params.name)
    {
        init(params.diffuse, params.normal, params.roughnessMap, params.metallicMap, params.aoMap, params.color);

        if(!params.roughnessMap.isValid())
            setMatRoughness(params.roughness);
        if(!params.metallicMap.isValid())
            setMatMetallic(params.metallic);
    }

    //---------------------------------------------------------------------------
    //  Private Functions
    //---------------------------------------------------------------------------

    void PBRMaterial::init(TexturePtr texture, TexturePtr normal, TexturePtr roughnessMap, 
                           TexturePtr metallicMap, TexturePtr aoMap, const Color& color)
    {
        // Some default-values
        setMatUVScale(1.0f);
        setMatColor(color);

        // Defaults for PBR
        setMatDiffuseTexture(texture);
        setMatNormalMap(normal);
        setMatRoughnessMap(roughnessMap);
        setMatMetallicMap(metallicMap);
        setMatAOMap(aoMap);
        setMatMetallic(0.0f);
        setMatRoughness(1.0f);

        roughnessMap.isValid() ? setMatUseRoughnessMap(true) : setMatUseRoughnessMap(false);
        metallicMap.isValid() ? setMatUseMetallicMap(true)  : setMatUseMetallicMap(false);

        // Parallax Displacement mapping
        setMatDisplacementMap(nullptr);
        setMatDisplacementScale(0.0f);
        setMatDisplacementBias(0.0f);
    }

    //---------------------------------------------------------------------------
    //  Public Functions
    //---------------------------------------------------------------------------

    void PBRMaterial::setMatDiffuseTexture(TexturePtr diffuse)
    {
        MappedValues::setTexture(SHADER_DIFFUSE_MAP_NAME, diffuse.isValid() ? diffuse : TEXTURE_GET(TEX_DEFAULT));
    }

    void PBRMaterial::setMatNormalMap(TexturePtr normalMap)
    {
        MappedValues::setTexture(SHADER_NORMAL_MAP_NAME, normalMap.isValid() ? normalMap : TEXTURE_GET(TEX_DEFAULT_NORMAL));
    }

    void PBRMaterial::setMatDisplacementMap(TexturePtr dispMap)
    {
        MappedValues::setTexture(SHADER_DISPLACEMENT_MAP_NAME, dispMap.isValid() ? dispMap : TEXTURE_GET(TEX_DEFAULT_DISPLACEMENT));
    }

    void PBRMaterial::setMatColor(const Color& color)
    {
        MappedValues::setColor(SHADER_COLOR_NAME, color);
    }

    void PBRMaterial::setMatUVScale(float uvScale)
    {
        MappedValues::setFloat(SHADER_UV_SCALE_NAME, uvScale);
    }

    void PBRMaterial::setMatDisplacementScale(float scale)
    {
        MappedValues::setFloat(SHADER_DISPLACEMENT_SCALE_NAME, scale);
    }

    void PBRMaterial::setMatDisplacementBias(float bias)
    {
        MappedValues::setFloat(SHADER_DISPLACEMENT_BIAS_NAME, bias);
    }

    void PBRMaterial::setMatMetallic(float metallic)
    {
        setMatUseMetallicMap(false);
        MappedValues::setFloat(SHADER_METALLIC_NAME, metallic);
    }

    void PBRMaterial::setMatRoughness(float roughness)
    {
        setMatUseRoughnessMap(false);
        MappedValues::setFloat(SHADER_ROUGHNESS_NAME, roughness);
    }

    void PBRMaterial::setMatMetallicMap(TexturePtr metallicMap)
    {
        setMatUseMetallicMap(true);
        MappedValues::setTexture(SHADER_METALLIC_MAP_NAME, metallicMap.isValid() ? metallicMap : TEXTURE_GET(TEX_DEFAULT_METALLIC));
    }

    void PBRMaterial::setMatRoughnessMap(TexturePtr roughnessMap)
    {
        setMatUseRoughnessMap(true);
        MappedValues::setTexture(SHADER_ROUGHNESS_MAP_NAME, roughnessMap.isValid() ? roughnessMap : TEXTURE_GET(TEX_DEFAULT_ROUGHNESS));
    }
    
    void PBRMaterial::setMatAOMap(TexturePtr aoMap)
    {
        MappedValues::setTexture(SHADER_AO_MAP_NAME, aoMap.isValid() ? aoMap : TEXTURE_GET(TEX_DEFAULT_AO_MAP));
    }

    float PBRMaterial::getMatMetallic()
    {
        return MappedValues::getFloat(SHADER_METALLIC_NAME);
    }
    
    float PBRMaterial::getMatRoughness()
    {
        return MappedValues::getFloat(SHADER_ROUGHNESS_NAME);
    }

    Color PBRMaterial::getMatColor()
    {
        return MappedValues::getColor(SHADER_COLOR_NAME);
    }

    //---------------------------------------------------------------------------
    //  Private Functions
    //---------------------------------------------------------------------------

    void PBRMaterial::setMatUseRoughnessMap(bool useMaps)
    {
        m_hasRoughnessMap = useMaps;
        MappedValues::setInt(SHADER_USE_ROUGHNESS_MAP, useMaps ? 1 : 0);
    }

    void PBRMaterial::setMatUseMetallicMap(bool useMaps)
    {
        m_hasMetallicMap = useMaps;
        MappedValues::setInt(SHADER_USE_METALLIC_MAP, useMaps ? 1 : 0);
    }

}