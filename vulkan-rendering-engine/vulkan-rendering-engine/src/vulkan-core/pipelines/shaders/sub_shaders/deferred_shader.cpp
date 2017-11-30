//#include "deferred_shader.h"
//
//#include "vulkan-core/data/material/texture/cubemap.hpp"
//#include "vulkan-core/scene_graph/scene_graph.h"
//#include "vulkan-core/rendering_engine.h"
//
//#include <assert.h>
//
//namespace ENGINE
//{
//
//    //---------------------------------------------------------------------------
//    //  Defines
//    //---------------------------------------------------------------------------
//
//    // Has to be the same as in shader
//    #define MAX_DIRECTIONAL_LIGHTS 4
//    #define MAX_POINT_LIGHTS 1024
//    #define MAX_SPOT_LIGHTS 32
//    #define MAX_NUM_SHADOW_MAPS_DIR_AND_SPOT 12
//    #define MAX_NUM_SHADOW_MAPS_POINT 8
//
//    // Offset defines for the different light types (because all of them are stored in one big buffer)
//    #define DIRECTIONAL_LIGHT_OFFSET sizeof(Vec4f)
//    #define POINT_LIGHT_OFFSET DIRECTIONAL_LIGHT_OFFSET + MAX_DIRECTIONAL_LIGHTS * sizeof(DirectionalLightData)
//    #define SPOT_LIGHT_OFFSET POINT_LIGHT_OFFSET + MAX_POINT_LIGHTS * sizeof(PointLightData)
//
//    //---------------------------------------------------------------------------
//    //  Constructor
//    //---------------------------------------------------------------------------
//
//    DeferredShader::DeferredShader(const std::string& name, const std::string& filePath, PipelineType pipeline,
//                                   const std::vector<ShaderStage>& shaderStages)
//        : Shader(name, filePath, pipeline, shaderStages, nullptr)
//    {
//        // Temporary solution for updating the descriptor-set with an "empty"-tex (validation layers throw an error if not updated)
//        emptyTex = new Texture("white.dds");
//        emptyCubemap = new Cubemap("cubemap_black.ktx");
//
//        for(int i = 0; i < MAX_NUM_SHADOW_MAPS_DIR_AND_SPOT; i++)
//            setTexture("ShadowMap", emptyTex, i);
//        
//        for (int i = 0; i < MAX_NUM_SHADOW_MAPS_POINT; i++)
//            setTexture("ShadowMapPointLight", emptyCubemap, i);
//    }
//
//    DeferredShader::~DeferredShader()
//    {
//        delete emptyTex;
//        delete emptyCubemap;
//    }
//
//    //---------------------------------------------------------------------------
//    //  Public Methods
//    //---------------------------------------------------------------------------
//
//    // Update uniforms in this shader
//    void DeferredShader::updateUniforms(RenderingEngine* renderer)
//    {
//        // Update camera position
//        setVec3f("Camera", renderer->getCamera()->getWorldPosition());
//
//        // Update lights
//        std::vector<DirectionalLightData>   directionalLights;
//        std::vector<PointLightData>         pointLights;
//        std::vector<SpotLightData>          spotLights;
//
//        shadowMapViewProjections.clear();
//        activeShadowsFromPointLights = 0;
//
//        for (auto& light : SceneGraph::getVisibleLights())
//        {
//            switch (light->getLightType())
//            {
//            case Light::DirectionalLight:
//                addDirectionalLight(directionalLights, light);
//                break;
//            case Light::PointLight:
//                addPointLight(pointLights, light);
//                break;
//            case Light::SpotLight:
//                addSpotLight(spotLights, light);
//                break;
//            }
//        }
//
//        // Update light view-projections and the current amount of shadow-maps for directional- and spotlights
//        if(shadowMapViewProjections.size() != 0)
//            setUniformBuffer("ShadowMapViewProjection", shadowMapViewProjections.data());
//
//        // Update light counts
//        LightCounts lightCounts;
//        lightCounts.NUM_DIRECTIONAL_LIGHTS = static_cast<int>(directionalLights.size());
//        lightCounts.NUM_POINT_LIGHTS = static_cast<int>(pointLights.size());
//        lightCounts.NUM_SPOT_LIGHTS = static_cast<int>(spotLights.size());
//        assert(lightCounts.NUM_DIRECTIONAL_LIGHTS <= MAX_DIRECTIONAL_LIGHTS && 
//               lightCounts.NUM_POINT_LIGHTS <= MAX_POINT_LIGHTS && 
//               lightCounts.NUM_SPOT_LIGHTS <= MAX_SPOT_LIGHTS);
//        setUniformBuffer("LightCounts", &lightCounts);
//
//        // Update the lighting uniforms if necessary
//        if (lightCounts.NUM_DIRECTIONAL_LIGHTS != 0)
//            setUniformBuffer("Lighting", directionalLights.data(), DIRECTIONAL_LIGHT_OFFSET, lightCounts.NUM_DIRECTIONAL_LIGHTS * sizeof(DirectionalLightData));
//        if (lightCounts.NUM_POINT_LIGHTS != 0)
//            setUniformBuffer("Lighting", pointLights.data(), POINT_LIGHT_OFFSET, lightCounts.NUM_POINT_LIGHTS * sizeof(PointLightData));
//        if (lightCounts.NUM_SPOT_LIGHTS != 0)
//            setUniformBuffer("Lighting", spotLights.data(), SPOT_LIGHT_OFFSET, lightCounts.NUM_SPOT_LIGHTS * sizeof(SpotLightData));
//    }
//
//    //---------------------------------------------------------------------------
//    //  Private Methods
//    //---------------------------------------------------------------------------
//
//    // Add a new "DirectionalLightData"-struct filled with the information from the given light to the given list
//    void DeferredShader::addDirectionalLight(std::vector<DirectionalLightData>& directionalLights, Light* light)
//    {
//        DirectionalLight* directionalLight = dynamic_cast<DirectionalLight*>(light);
//
//        int samplerIndex = -1;
//      
//        // If Shadows from this light are enabled, add the view-projection from it, update the descriptor-set and set the samplerIndex
//        if (VulkanBase::getSettings().renderShadows && light->shadowsEnabled())
//        {
//            samplerIndex = static_cast<int>(shadowMapViewProjections.size());
//            setTexture("ShadowMap", light->getShadowMapTex(), samplerIndex);
//            shadowMapViewProjections.push_back(light->getShadowViewProjection());
//        }
//
//        DirectionalLightData directionalLightData;
//        directionalLightData.base.color         = directionalLight->getColor().getRGB();
//        directionalLightData.base.intensity     = directionalLight->getIntensity();
//        directionalLightData.base.position      = directionalLight->getWorldPosition();
//        directionalLightData.base.samplerIndex  = samplerIndex;
//        directionalLightData.direction          = directionalLight->getDirection();
//        
//        directionalLights.push_back(directionalLightData);
//    }
//
//    // Add a new "PointLightData"-struct filled with the information from the given light to the given list
//    void DeferredShader::addPointLight(std::vector<PointLightData>& pointLights, Light* light)
//    {
//        PointLight* pointLight = dynamic_cast<PointLight*>(light);
//
//        int samplerIndex = -1;
//
//        // If Shadows from this light are enabled, update the descriptor-set and set the samplerIndex
//        if (VulkanBase::getSettings().renderShadows && light->shadowsEnabled())
//        {
//            samplerIndex = activeShadowsFromPointLights++;
//            setTexture("ShadowMapPointLight", light->getShadowMapTex(), samplerIndex);
//        }
//
//        PointLightData pointLightData;
//        pointLightData.base.color        = pointLight->getColor().getRGB();
//        pointLightData.base.intensity    = pointLight->getIntensity();
//        pointLightData.base.position     = pointLight->getWorldPosition();
//        pointLightData.base.samplerIndex = samplerIndex;
//        pointLightData.attenuation       = pointLight->getAttenuation();
//        pointLightData.range             = pointLight->getRange();
//
//        pointLights.push_back(pointLightData);
//    }
//
//    // Add a new "SpotLightData"-struct filled with the information from the given light to the given list
//    void DeferredShader::addSpotLight(std::vector<SpotLightData>& spotLights, Light* light)
//    {
//        SpotLight* spotLight = dynamic_cast<SpotLight*>(light);
//
//        int samplerIndex = -1;
//
//        // If Shadows from this light are enabled, add the view-projection from it, update the descriptor-set and set the samplerIndex
//        if (VulkanBase::getSettings().renderShadows && light->shadowsEnabled())
//        {
//            samplerIndex = static_cast<int>(shadowMapViewProjections.size());
//            setTexture("ShadowMap", light->getShadowMapTex(), samplerIndex);
//            shadowMapViewProjections.push_back(light->getShadowViewProjection());
//        }
//
//        SpotLightData spotLightData;
//        spotLightData.pointLight.base.color         = spotLight->getColor().getRGB();
//        spotLightData.pointLight.base.intensity     = spotLight->getIntensity();
//        spotLightData.pointLight.base.position      = spotLight->getWorldPosition();
//        spotLightData.pointLight.base.samplerIndex  = samplerIndex;
//        spotLightData.pointLight.attenuation        = spotLight->getAttenuation();
//        spotLightData.pointLight.range              = spotLight->getRange();
//        spotLightData.direction                     = spotLight->getDirection();
//        spotLightData.cutoff                        = spotLight->getCutoff();
//
//        spotLights.push_back(spotLightData);
//    }
//
//}