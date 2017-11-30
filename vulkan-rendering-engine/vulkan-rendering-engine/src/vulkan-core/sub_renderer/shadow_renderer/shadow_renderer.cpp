#include "shadow_renderer.h"

#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"
#include "vulkan-core/pipelines/renderpass/renderpass.h"
#include "vulkan-core/scene_graph/scene_manager.h"
#include "vulkan-core/data/lighting/light.h"
#include "vulkan-core/vkTools/vk_tools.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    VkFormat    ShadowRenderer::colorFormat;
    
    Renderpass* ShadowRenderer::renderpass;
    Renderpass* ShadowRenderer::renderpassGaussianBlur;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    ShadowRenderer::ShadowRenderer(RenderingEngine* r)
        : SubRenderer(r, true)
    {
        VkFormat depthFormat = VK_FORMAT_D16_UNORM;
        colorFormat = VK_FORMAT_R32G32_SFLOAT;

        prepareRenderpass(colorFormat, depthFormat);

        ShaderParams params("ShadowMap", "/shaders/shadowMap", PipelineType::Shadowmap, renderpass);
        shadowMapShader = SHADER(params);

        ShaderParams params2("ShadowMapPointLight", "/shaders/shadowMapPointLight", PipelineType::Shadowmap, renderpass);
        shadowMapShaderPointLight = SHADER(params2);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    ShadowRenderer::~ShadowRenderer()
    {
        delete renderpass;
        delete renderpassGaussianBlur;
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Record the command buffer which renders all shadowmaps.
    void ShadowRenderer::recordCommandBuffer(uint32_t frameDataIndex)
    {
        cmdBuffers[frameDataIndex]->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        {
            // Check if the amount of static-lights to last frame has changed. If so, render all shadow-maps from static-lights again
            renderShadowmapsFromStaticLights(cmdBuffers[frameDataIndex].get());

            // Render the shadow-map for each enabled light
            for (const auto& light : SceneManager::getCurrentScene()->getLights())
            {
                // Skip this light if shadows are not enabled or the light is static
                if(!light->shadowsEnabled() || light->isStatic() || !light->isActive())
                    continue;

                if(light->getLightType() == Light::PointLight)
                    renderShadowMapFromPointLight(cmdBuffers[frameDataIndex].get(), dynamic_cast<PointLight*>(light));
                else
                    renderShadowMapFromLight(cmdBuffers[frameDataIndex].get(), light);
            }
        }
        cmdBuffers[frameDataIndex]->end();
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Check if the amount of static-lights to last frame has changed. If so, render all shadow-maps from static-lights again
    void ShadowRenderer::renderShadowmapsFromStaticLights(CommandBuffer* commandBuffer)
    {
        static int numStaticLights = 0;

        int currentNumStaticLights = 0;
        for (const auto& light : SceneManager::getCurrentScene()->getLights())
            if(light->isStatic())
                currentNumStaticLights++;

        if (numStaticLights != currentNumStaticLights)
        {
            for (const auto& light : SceneManager::getCurrentScene()->getLights())
            {
                if (light->isStatic() && light->shadowsEnabled())
                {
                    if (light->getLightType() == Light::PointLight)
                        renderShadowMapFromPointLight(commandBuffer, dynamic_cast<PointLight*>(light));
                    else
                        renderShadowMapFromLight(commandBuffer, light);
                }
            }

            numStaticLights = currentNumStaticLights;
        }
    }

    // Record the commands of rendering a shadow-map from the given light into the given cmd
    void ShadowRenderer::renderShadowMapFromLight(CommandBuffer* commandBuffer, Light* light)
    {
        VkCommandBuffer cmd = commandBuffer->get();

        // Update dynamic viewport + scissor state
        Framebuffer* shadowFBO = light->shadowInfo->framebuffer;
        commandBuffer->setViewport(shadowFBO);
        commandBuffer->setScissor(shadowFBO);

        // Begin renderpass
        renderpass->begin(cmd, shadowFBO);

        // Bind shadow-map pipeline
        shadowMapShader->bind(cmd);

        // Update per light data through push-constant
        Mat4f lightViewProjection = light->getShadowViewProjection();

        // Offset of 64 (First matrice is for per-object data)
        commandBuffer->pushConstants(shadowMapShader->getPipelineLayout()->get(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(Mat4f), sizeof(Mat4f), &lightViewProjection);

        // Render all objects within light-frustum for spot-lights or all for dir-lights
        std::vector<Renderable*> renderables;
        if (light->getLightType() == Light::SpotLight)
        {
            const float radius = dynamic_cast<SpotLight*>(light)->getRange();
            renderables = SceneManager::getCurrentScene()->getRenderablesWithinRadius(light->getWorldPosition(), radius);
        }
        else
            renderables = SceneManager::getCurrentScene()->getRenderables();

        light->getShadowCamera()->render(cmd, shadowMapShader, renderables, true);

        renderpass->end(cmd);

        // Blur Variance-Shadowmap if it is enabled for this light
        if (light->shadowInfo->hBlur != nullptr)
        {
            // Blur First-Time
            light->shadowInfo->hBlur->record(commandBuffer, { shadowFBO }, nullptr);

            // Blur Second-Time back to light-framebuffer
            light->shadowInfo->vBlur->record(commandBuffer, shadowFBO, { light->shadowInfo->hBlur->getOutputFramebuffer() });
        }
    }

    // Record the commands of rendering a shadow-map from the given point-light into the given cmd
    void ShadowRenderer::renderShadowMapFromPointLight(CommandBuffer* cmd, PointLight* light)
    {
        // Get Framebuffer-color attachment. We will render in here and copy the resulting cubemap-face to the cubemap-image from the point-light
        Framebuffer* shadowFBO = light->shadowInfo->framebuffer;

        // Determine visible objects for this point-light
        const Point3f& lightPos = light->getWorldPosition();
        const float radius = light->getRange();
        auto& renderables = SceneManager::getCurrentScene()->getRenderablesWithinRadius(lightPos, radius);

        // Bind Light-Descriptor-Set
        light->bind(cmd->get(), shadowMapShaderPointLight->getPipelineLayout());

        vkTools::renderCubemap(cmd, renderpass, shadowMapShaderPointLight, shadowFBO, light->getCubemapImage(), lightPos,
            [&](VkCommandBuffer cmd, Mat4f view) {
                // Set view-matrix from the shadow-camera
                light->getShadowCamera()->setViewMatrix(view);

                // Offset of 64 (First matrix is for per-object data)
                shadowMapShaderPointLight->pushConstant(cmd, sizeof(Mat4f), sizeof(Mat4f), &light->getShadowViewProjection());

                // Render all objects within light frustum
                light->getShadowCamera()->render(cmd, shadowMapShaderPointLight, renderables, true);
        });
    }
  
    // Prepare a separate render pass for rendering the shadow-maps
    void ShadowRenderer::prepareRenderpass(const VkFormat& colorFormat, const VkFormat& depthFormat)
    {
        // Color attachment
        VkAttachmentDescription colorAttachment, depthAttachment;
        colorAttachment.flags            = 0;
        colorAttachment.format           = colorFormat;
        colorAttachment.samples          = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Depth attachment
        depthAttachment.flags            = 0;
        depthAttachment.format           = depthFormat;
        depthAttachment.samples          = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        renderpass = new Renderpass(device, { colorAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
                                            { depthAttachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });

        // Create a renderpass only with a color-attachment
        renderpassGaussianBlur = new Renderpass(device, { colorAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
    }


}