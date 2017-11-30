#ifndef SHADOW_RENDERER_H_
#define SHADOW_RENDERER_H_

#include "vulkan-core/resource_manager/resource.hpp"
#include "vulkan-core/sub_renderer/sub_renderer.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class CommandBuffer;
    class PointLight;
    class Shader;
    class Light;

    //---------------------------------------------------------------------------
    //  ShadowRenderer-Class
    //---------------------------------------------------------------------------

    class ShadowRenderer : public SubRenderer
    {
    public:
        ShadowRenderer(RenderingEngine* renderingEngine);
        ~ShadowRenderer();

        // Record the command buffer which renders all shadowmaps.
        void recordCommandBuffer(uint32_t frameDataIndex) override;

        // Statics Methods, accessed from the "Light-Superclass" to create an appropriate ShadowMap-Framebuffer
        static VkFormat     getColorFormat(){ return colorFormat; }
        static Renderpass*  getRenderPass(){ return renderpass; }
        static Renderpass*  getRenderPassGaussianBlur() { return renderpassGaussianBlur; }

    private:
        // forbid copy and copy assignment
        ShadowRenderer(const ShadowRenderer& shadowRenderer);
        ShadowRenderer& operator=(const ShadowRenderer& shadowRenderer) {};

        // This shader will be used to render the shadow-maps from dir- & spot-lights
        ShaderPtr           shadowMapShader;

        // This shader will be used to render the shadow-maps from point-lights
        ShaderPtr           shadowMapShaderPointLight;

        // Color format of the shadow-maps. The Point-Light needs to know this to create a cubemap.
        static VkFormat     colorFormat;

        // The renderpass for rendering the shadow-maps
        static Renderpass*  renderpass;

        // The second renderpass for rendering the shadow-map into an 
        // offscreen-framebuffer without a depth-attachment to apply a GaussianBlur
        static Renderpass*  renderpassGaussianBlur;

        // Create the renderpass for rendering shadow-maps
        void prepareRenderpass(const VkFormat& colorFormat, const VkFormat& depthFormat);

        // Check if the amount of static-lights to last frame has changed. If so, render all shadow-maps from static-lights again
        void renderShadowmapsFromStaticLights(CommandBuffer* commandBuffer);

        // Record the commands of rendering a shadow-map from the given light into the given cmd
        void renderShadowMapFromLight(CommandBuffer* commandBuffer, Light* light);

        // Record the commands of rendering a shadow-map from the given point-light into the given cmd
        void renderShadowMapFromPointLight(CommandBuffer* commandBuffer, PointLight* light);
    };

}

#endif // !SHADOW_RENDERER_H_
