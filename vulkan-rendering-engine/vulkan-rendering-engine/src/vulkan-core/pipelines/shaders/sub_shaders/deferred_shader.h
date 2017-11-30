//#ifndef DEFERRED_SHADER_H_
//#define DEFERRED_SHADER_H_
//
//#include "../shader.h"
//#include "vulkan-core/data/lighting/light.h"
//
//namespace ENGINE
//{
//    class Cubemap;
//
//    // Mimic the structs in the shader
//    struct LightCounts
//    {
//        int NUM_DIRECTIONAL_LIGHTS;
//        int NUM_POINT_LIGHTS;
//        int NUM_SPOT_LIGHTS;
//    };
//
//    struct BaseLight
//    {
//        Vec3f color;
//        float intensity;
//        Vec3f position;
//        int samplerIndex;   // ShadowMapSampler-Index. -1 if shadow from this light are disabled.
//    };
//
//    struct DirectionalLightData
//    {
//        BaseLight   base;
//        Vec3f       direction;
//        float       padding;      // Necessary
//    };
//
//    struct PointLightData
//    {
//        BaseLight   base;
//        Vec3f       attenuation;
//        float       range;
//    };
//
//    struct SpotLightData
//    {
//        PointLightData  pointLight;
//        Vec3f           direction;
//        float           cutoff;
//    };
//
//    class DeferredShader : public Shader
//    {
//    public:
//        DeferredShader(const std::string& name, const std::string& filePath, PipelineType pipeline, const std::vector<ShaderStage>& shaderStages);
//        ~DeferredShader();
//
//        void updateUniforms(RenderingEngine* renderer) override;
//
//    private:
//        void addDirectionalLight(std::vector<DirectionalLightData>& directionalLights, Light* light);
//        void addPointLight(std::vector<PointLightData>& pointLights, Light* light);
//        void addSpotLight(std::vector<SpotLightData>& spotLights, Light* light);
//
//        // Temporary solution for updating the descriptor-set with an "empty"-tex (validation layers throw an error if not updated)
//        Texture* emptyTex;
//        Cubemap* emptyCubemap;
//
//        // Vector used to pass all view-projection from all active-lights (directional + spot) with shadows to the gpu
//        // The Point-Light itself dont need a view-projection.
//        std::vector<Mat4f> shadowMapViewProjections;
//
//        // Used to give an active point-light with shadows an sampler-index
//        // Point-lights use an different array of Samplers in the shader
//        uint32_t activeShadowsFromPointLights = 0;
//    };
//
//}
//
//
//#endif // !DEFERRED_SHADER_H_
