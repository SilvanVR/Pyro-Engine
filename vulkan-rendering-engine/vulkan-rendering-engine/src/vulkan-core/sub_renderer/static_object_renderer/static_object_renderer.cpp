#include "static_object_renderer.h"

#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"
#include "vulkan-core/scene_graph/scene_graph.h"
#include "vulkan-core/rendering_engine.h"

#include "vulkan-core/vkTools/vk_tools.h"

namespace ENGINE
{


    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    StaticObjectRenderer::StaticObjectRenderer(RenderingEngine* r)
        : SubRenderer(r, false)
    {
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Return a secondary-cmd for the given frame-resource index. Rerecord if necessary.
    // Return nullptr if there is no static-object to render.
    VkCommandBuffer StaticObjectRenderer::getSecCmd(uint32_t index)
    {
        static uint32_t lastStaticNodeCount = 0;

        uint32_t currentStaticNodeCount = static_cast<uint32_t>(SceneGraph::getStaticNodes().size());

        if (currentStaticNodeCount != lastStaticNodeCount)
        {
            // Num static objects has changed, rerecord cmd-buffers
            preRecordCommandBuffers();
            lastStaticNodeCount = currentStaticNodeCount;
        }

        if (currentStaticNodeCount != 0)
            return cmdBuffers[index].get();
        else
            return nullptr;
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Record the draw-commands into the command buffers
    void StaticObjectRenderer::preRecordCommandBuffers()
    {
        vkDeviceWaitIdle(device);
        VkCommandBufferUsageFlags cmdUsage = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        // All framebuffers have the same dimension
        uint32_t width = frameResources[0].framebuffer->getWidth();
        uint32_t height = frameResources[0].framebuffer->getHeight();

        VkViewport viewport = { 0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
        VkRect2D   scissor = { { 0,0 },{ width, height } };

        VkCommandBufferInheritanceInfo inheritanceInfo = {};
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritanceInfo.pNext = nullptr;
        inheritanceInfo.renderPass = renderingEngine->getRenderpass()->get();
        inheritanceInfo.subpass = 0;

        for (unsigned int i = 0; i < cmdBuffers.size(); i++)
        {
            inheritanceInfo.framebuffer = frameResources[i].framebuffer->get();

            cmdBuffers[i].begin(cmdUsage, &inheritanceInfo);
            {
                VkCommandBuffer cmd = cmdBuffers[i].get();

                // Update dynamic viewport state
                vkCmdSetViewport(cmd, 0, 1, &viewport);

                // Update dynamic scissor state
                vkCmdSetScissor(cmd, 0, 1, &scissor);

                // Bind View-Projection Set once used by all following Pipelines
                renderingEngine->getDescriptorSet()->bind(cmd, Shader::get("Phong")->getPipelineLayout());

                // Draw all Meshes with different shaders aka pipelines
                for (auto& shader : Shader::getAllActiveStaticShaders())
                {
                    // Bind pipeline and descriptor-sets associated with the shaders from that pipe
                    shader->bind(cmd);

                    for (auto& material : shader->getMaterials())
                    {
                        const std::vector<Renderable*> renderables = material->getStaticRenderables();

                        if (renderables.size() != 0)
                        {
                            // Bind the material descriptor-set to the shader
                            material->bind(cmd);

                            // Now render all STATIC renderables.
                            for (auto& object : renderables)
                                object->render(cmd, shader);
                        }
                    }
                }

            }
            cmdBuffers[i].end();
        }

    }

    //---------------------------------------------------------------------------
    //  OnSizeChanged - Callback
    //---------------------------------------------------------------------------

    // Called from the Rendering-Engine when the window size has changed.
    void StaticObjectRenderer::onSizeChanged()
    {
        preRecordCommandBuffers();
    }

}