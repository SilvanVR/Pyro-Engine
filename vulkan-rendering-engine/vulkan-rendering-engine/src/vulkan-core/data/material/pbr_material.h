#ifndef PBR_MATERIAL_H_
#define PBR_MATERIAL_H_

#include "material.h"


namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines (Matching shader names)
    //---------------------------------------------------------------------------

    #define SHADER_DIFFUSE_MAP_NAME         "diffuse"
    #define SHADER_NORMAL_MAP_NAME          "normalMap"
    #define SHADER_DISPLACEMENT_MAP_NAME    "dispMap"
    #define SHADER_COLOR_NAME               "color"
    #define SHADER_UV_SCALE_NAME            "uvScale"
    #define SHADER_DISPLACEMENT_SCALE_NAME  "dispScale"
    #define SHADER_DISPLACEMENT_BIAS_NAME   "dispBias"
    #define SHADER_METALLIC_NAME            "metallic"
    #define SHADER_ROUGHNESS_NAME           "roughness"
    #define SHADER_ROUGHNESS_MAP_NAME       "roughnessMap"
    #define SHADER_METALLIC_MAP_NAME        "metallicMap"
    #define SHADER_AO_MAP_NAME              "aoMap"
    #define SHADER_USE_ROUGHNESS_MAP        "useRoughnessMap"
    #define SHADER_USE_METALLIC_MAP         "useMetallicMap"

    //---------------------------------------------------------------------------
    //  Structs
    //---------------------------------------------------------------------------

    struct PBRMaterialParams
    {
        PBRMaterialParams(TexturePtr diffuse, const std::string& name)
            : PBRMaterialParams(diffuse, nullptr, nullptr, nullptr, nullptr, Color::WHITE, name) {}

        PBRMaterialParams(TexturePtr diffuse, TexturePtr normal = nullptr, TexturePtr roughnessMap = nullptr,
            TexturePtr metallicMap = nullptr, const Color& color = Color::WHITE, const std::string& name = "PBRMaterial")
            : PBRMaterialParams(diffuse, normal, roughnessMap, metallicMap, nullptr, color, name) {}

        PBRMaterialParams(TexturePtr _diffuse, TexturePtr _normal, TexturePtr _roughnessMap, TexturePtr _metallicMap,
                          TexturePtr _aoMap, const Color& _color = Color::WHITE, const std::string& _name = "PBRMaterial")
            : diffuse(_diffuse), normal(_normal), roughnessMap(_roughnessMap), metallicMap(_metallicMap), aoMap(_aoMap), 
            color(_color), name(_name), roughness(1.0f), metallic(0.0f) {}

        PBRMaterialParams(TexturePtr _diffuse, float _roughness, float _metallic, const Color& _color = Color::WHITE,
                          const std::string& _name = "PBRMaterial")
            : diffuse(_diffuse), roughness(_roughness), metallic(_metallic), color(_color), name(_name) {}

        TexturePtr  diffuse;
        TexturePtr  normal; 
        TexturePtr  roughnessMap;
        TexturePtr  metallicMap;
        float       roughness;
        float       metallic;
        TexturePtr  aoMap;
        Color       color;
        std::string name;
    };


    //---------------------------------------------------------------------------
    //  PBRMaterial class
    //---------------------------------------------------------------------------

    class PBRMaterial : public Material
    {

    public:
        // Create a material using the pbr shader.
        // The given texture is the diffuse-texture.
        // It sets default values for all necessary fields (metallic, roughness etc.)
        PBRMaterial(const PBRMaterialParams& params);

        ~PBRMaterial() {}

        // Basic functions for settings all shader relevant fields for a material of this kind
        void setMatDiffuseTexture(TexturePtr diffuse);
        void setMatNormalMap(TexturePtr normalMap);
        void setMatDisplacementMap(TexturePtr dispMap);
        void setMatColor(const Color& color);
        void setMatUVScale(float uvScale);
        void setMatDisplacementScale(float scale);
        void setMatDisplacementBias(float bias);

        void setMatMetallic(float metallic);
        void setMatRoughness(float roughness);
        void setMatMetallicMap(TexturePtr metallicMap);
        void setMatRoughnessMap(TexturePtr roughnessMap);
        void setMatAOMap(TexturePtr aoMap);

        // Getter's
        float getMatMetallic();
        float getMatRoughness();
        Color getMatColor();
        bool hasMetallicMap() const { return m_hasMetallicMap; }
        bool hasRoughnessMap() const { return m_hasRoughnessMap; }

    private:
        void init(TexturePtr texture, TexturePtr normal, TexturePtr roughnessMap, 
                  TexturePtr metallicMap, TexturePtr aoMap, const Color& color);

        void setMatUseRoughnessMap(bool useMap);
        void setMatUseMetallicMap(bool useMap);

        bool m_hasRoughnessMap;
        bool m_hasMetallicMap;
    };

}



#endif // !PBR_MATERIAL_H_

