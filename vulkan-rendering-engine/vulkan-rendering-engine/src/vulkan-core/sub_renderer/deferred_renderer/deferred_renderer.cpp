#include "deferred_renderer.h"

#include "vulkan-core/rendering_engine.h"
#include "vulkan-core/scene_graph/scene_graph.h"

namespace ENGINE
{


    DeferredRenderer::DeferredRenderer(RenderingEngine* r)
        : SubRenderer(r, true),
        loadRenderpass(r->loadRenderpass),
        dirLightShader("DirLightShader", "res/shaders/deferred_rendering/dir_light", PipelineType::Light),
        pointLightShader("PointLightShader", "res/shaders/deferred_rendering/point_light", PipelineType::Light),
        spotLightShader("SpotLightShader", "res/shaders/deferred_rendering/spot_light", PipelineType::Light),
        sphereMesh("res/models/sphere.obj")
    {

    }

    DeferredRenderer::~DeferredRenderer()
    {

    }

    void DeferredRenderer::record(uint32_t frameDataIndex, VkCommandBuffer cmd)
    {
        // Framebuffer with a color + depth/stencil attachment. Color + Depth will be loaded.
        loadRenderpass->begin(cmd, frameResources[frameDataIndex].framebuffer);
        {
            // Bind directional-light shader pipe and sets
            dirLightShader.bind(cmd);

            // Bind descriptor-set referencing the G-Buffer
            frameResources[frameDataIndex].gBuffer->bind(cmd, dirLightShader.getPipelineLayout());

            // Bind Camera-Descriptor-Set
            RenderingEngine::getCamera()->bind(cmd, dirLightShader.getPipelineLayout());

            for (const auto& dirLight : SceneGraph::getDirectionalLights())
            {
                // Bind Light-Descriptor-Set
                dirLight->bind(cmd, dirLightShader.getPipelineLayout());

                // Draw fullscreen quad for each dir-light
                vkCmdDraw(cmd, 3, 1, 0, 0);
            }

            // Bind Index- & Vertex-Buffer
            sphereMesh.bind(cmd);

            for (const auto& pointLight : SceneGraph::getVisiblePointLights())
            {
                // Bind point-light shader
                pointLightShader.bind(cmd);

                // Bind Light-Descriptor-Set
                pointLight->bind(cmd, pointLightShader.getPipelineLayout());

                // Update model-matrix through push-constant
                const Mat4f& worldMat = pointLight->getWorldMatrix();
                vkCmdPushConstants(cmd, pointLightShader.getPipelineLayout()->get(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4f), &worldMat);

                // Draw sphere scaled with range of the light
                sphereMesh.draw(cmd);
            }

            // Bind spot-light shader
            spotLightShader.bind(cmd);

            for (const auto& spotLight : SceneGraph::getVisibleSpotLights())
            {
                // Bind Light-Descriptor-Set
                spotLight->bind(cmd, spotLightShader.getPipelineLayout());

                // Update per light data through push-constant
                const Mat4f& worldMat = spotLight->getWorldMatrix();
                vkCmdPushConstants(cmd, pointLightShader.getPipelineLayout()->get(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4f), &worldMat);

                // Draw sphere scaled with range of the light
                sphereMesh.draw(cmd);
            }
        }
        vkCmdEndRenderPass(cmd);
    }

    void DeferredRenderer::recordWired(uint32_t frameDataIndex, VkCommandBuffer cmd)
    {
        // Draw a wired sphere around all lights
        sphereMesh.bind(cmd);
        for (const auto& light : SceneGraph::getVisibleLights())
        {
            // Update model-matrix through push-constant
            const Mat4f& worldMat = light->getWorldMatrix();
            vkCmdPushConstants(cmd, wireframeShader.getPipelineLayout()->get(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4f), &worldMat);

            // Draw sphere scaled with range of the light
            sphereMesh.draw(cmd);
        }
    }

    void DeferredRenderer::recordCommandBuffer(uint32_t frameDataIndex, Framebuffer* framebuffer)
    {

    }


    void DeferredRenderer::onSizeChanged()
    {

    }






}