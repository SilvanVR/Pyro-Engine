#include "rendering_engine.h"

#include "sub_renderer/post_processing_renderer/post_processing_renderer.h"
#include "vulkan-core/resource_manager/resource_manager.h"
#include "sub_renderer/shadow_renderer/shadow_renderer.h"
#include "vulkan-core/pipelines/renderpass/renderpass.h"
#include "scene_graph/nodes/renderables/renderable.h"
#include "sub_renderer/gui_renderer/gui_renderer.h"
#include "pipelines/shaders/forward_shader.h"
#include "data/material/basic_material.h"
#include "data/material/pbr_material.h"
#include "scene_graph/scene_manager.h"
#include "vkTools/vk_tools.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    Camera* RenderingEngine::camera = nullptr;    // Reference to the camera used for rendering

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    RenderingEngine::RenderingEngine(Window* window)
        : VulkanBase(window),
        renderingMode(ERenderingMode::LIT)
    {
        init();
    }

    // Creates and Initializes the rendering-engine. Renders into the given file-path with the specified dimensions
    RenderingEngine::RenderingEngine(const Vec2ui& resolution)
        : VulkanBase(resolution),
        renderingMode(ERenderingMode::LIT)
    {
        init();
    }

    // Initialize everything
    void RenderingEngine::init()
    {
        gBufferShader    = SHADER(SHADER_GBUFFER);
        solidShader      = SHADER(SHADER_SOLID);
        wireframeShader  = SHADER(SHADER_FW_WIREFRAME);
        dirLightShader   = SHADER(SHADER_DIR_LIGHT);
        pointLightShader = SHADER(SHADER_POINT_LIGHT);
        spotLightShader  = SHADER(SHADER_SPOT_LIGHT);

        subRenderer[GUI]         = new GUIRenderer(this);
        subRenderer[SHADOW]      = new ShadowRenderer(this);
        subRenderer[POSTPROCESS] = new PostProcessingRenderer(this);

        // Calls resetStateToDefault()
        SceneManager::init(this);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    RenderingEngine::~RenderingEngine()
    {
        vkDeviceWaitIdle(device0);
        SceneManager::destroy();
        for(auto& sr : subRenderer)
            delete sr.second; 
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void RenderingEngine::resetStateToDefault()
    {
        camera = nullptr;
        settings.renderShadows      = true;
        settings.renderGUI          = true;
        settings.doPostProcessing   = true;

        for(auto& mv : MappedValues::mappedValues)
            mv->lastFlushFrameDataIndex = -1;

        setClearColor(Color::BLACK);
        setTimeScale(1.0f);
        setRenderingMode(ERenderingMode::LIT);
        setAmbientIntensity(0.1f);
        setEnvMapSpecularIntensity(1.0f);
        setResolutionMod(1.0f);
        setUseIBL(true);
        setBRDFLUT(TEXTURE_GET(TEX_DEFAULT_BRDF_LUT));
        setAmbientIrradianceMap(CUBEMAP_GET(CUBEMAP_DEFAULT_IRRADIANCE));
        setEnvironmentMap(CUBEMAP_GET(CUBEMAP_DEFAULT_ENVIRONMENT));
        setAlphaDiscardThreshold(0.5f);
        setRenderBoundingBoxes(false);
    }

    // Update scene-graph
    void RenderingEngine::update(float delta)
    {
        // Update current scene
        SceneManager::update(delta * timeScale);

        // Update all subrenderer
        for(auto& sr : subRenderer)
            sr.second->update(delta * timeScale);
    }

    // Records graphics work, submit it to the gpu and present the result
    void RenderingEngine::draw()
    {
        // Calculate new frame-data-index
        frameDataIndex = nextFrameDataIndex;
        nextFrameDataIndex = (frameDataIndex + 1) % frameResources.size();
        currentFrameData = &frameResources[frameDataIndex];

        // Wait on the frame-data fence if necessary. This guarantees that everything needed this frame can safely be reused
        currentFrameData->fence->wait(UINT64_MAX);
        currentFrameData->fence->reset();

        // Record commands into command-buffers
        recordCommandBuffers();

        // Gather all Command-Buffers and submit them all in once
        std::vector<const CommandBuffer*> commandBuffers;
        {
            // Add Shadow-Map Rendering Command Buffer
            if (settings.renderShadows)
                commandBuffers.push_back(subRenderer[SHADOW]->getCMD(frameDataIndex));

            // Add the Primary-CMD which renders the scene
            commandBuffers.push_back(frameResources[frameDataIndex].primaryCmd.get());

            // Add Post-Processing Command Buffer
            commandBuffers.push_back(subRenderer[POSTPROCESS]->getCMD(frameDataIndex));

            // Add GUI Command Buffer
            if (settings.renderGUI)
                commandBuffers.push_back(subRenderer[GUI]->getCMD(frameDataIndex));
        }

        // Submit all Command Buffer in the List at once
        CommandBuffer::submit(graphicQueue, commandBuffers);

        // Last image in which the engine has rendered
        VulkanImage& renderedImage = subRenderer[POSTPROCESS]->getOutputFramebuffer()->getColorImage();

        // Retrieve data from the gpu and call the callback with that data if the callback is valid
        if (renderingFinishedCallback)
            getRenderedDataAndCallCallback(renderedImage);

        // Submit rendered result to the presentation-engine using the color-image
        // from the last post-processed framebuffer (on which the gui was rendered aswell)
        if (hasWindow())
            VulkanBase::submitFrame(renderedImage);
        else
        {
            // Submit an empty command-buffer because the frame-data fence has to be signaled
            currentFrameData->blitCmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            currentFrameData->blitCmd->end();
            currentFrameData->blitCmd->submit(graphicQueue, currentFrameData->fence);
        }
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Record all command-buffers across all renderers
    void RenderingEngine::recordCommandBuffers()
    {
        // Check if a camera exists
        if (camera == nullptr)
            Logger::Log("No Camera is used. Please call setCamera() before any other function on the renderer", LOGTYPE_ERROR);

        if (settings.renderShadows)
            subRenderer[SHADOW]->recordCommandBuffer(frameDataIndex);

        // Record cmd for the scene.
        recordSceneCommandBuffer();

        // We have to do at least one post-process step, which rescales the scene-rendering
        // back to the window-resolution (if we rendered into a lower/higher resolution)
        Framebuffer* framebuffer = frameResources[frameDataIndex].forwardFramebuffer;
        subRenderer[POSTPROCESS]->recordCommandBuffer(frameDataIndex, framebuffer);

        // Render the GUI onto the last framebuffer from the Post-Processing
        if (settings.renderGUI)
            subRenderer[GUI]->recordCommandBuffer(frameDataIndex, subRenderer[POSTPROCESS]->getOutputFramebuffer());
    }

    // Record primary command-buffer which renders the scene
    void RenderingEngine::recordSceneCommandBuffer()
    {
        VkCommandBufferUsageFlags cmdUsage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (renderingMode == ERenderingMode::LIT || renderingMode == ERenderingMode::UNLIT)
        {
            // Record commands into the primary command-buffer using the deferred-rendering method
            currentFrameData->primaryCmd->begin(cmdUsage);
            {
                VkCommandBuffer cmd = currentFrameData->primaryCmd->get();

                // Update dynamic viewport+scissor state
                Framebuffer* fbo = frameResources[frameDataIndex].mrtFramebuffer;
                currentFrameData->primaryCmd->setViewport(fbo);
                currentFrameData->primaryCmd->setScissor(fbo);

                // Begin renderpass
                mrtRenderpass->begin(cmd, frameResources[frameDataIndex].mrtFramebuffer);

                // PreProcess
                if (preProcessingEnabled)
                {
                    preProcessShader->bind(cmd);
                    vkCmdDraw(cmd, 3, 1, 0, 0);
                }

                // Bind gBuffer-shader pipeline and descriptor-set associated with the shaders from that pipe
                gBufferShader->bind(cmd);

                // Bind View-Projection Set
                camera->bind(cmd, gBufferShader->getPipelineLayout());

                for (auto& material : gBufferShader->getMaterialsFromCurrentScene())
                {
                    const std::vector<Renderable*>& renderables = material->getRenderablesFromCurrentScene();

                    if (renderables.size() != 0)
                    {
                        // Bind the material descriptor-set to the shader
                        material->bind(cmd);

                        // Render all objects with that material through the main camera
                        camera->render(cmd, gBufferShader, renderables, settings.cull);
                    }
                }

                mrtRenderpass->end(cmd);

                // Make sure GBuffer rendering has been finished before deferred lighting will be applied
                currentFrameData->primaryCmd->pipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                              VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

                // Do deferred-lighting.
                if (renderingMode == ERenderingMode::LIT)
                    recordDeferredLightingCommands(cmd);

                // Make sure lighting has been finished before the forward-rendering can happen
                currentFrameData->primaryCmd->pipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                              VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

                // Render all objects with unique shaders (ForwardShader-Objects)
                recordForwardCommands(cmd);
            }
            currentFrameData->primaryCmd->end();
        }
        else
        {
            // Record commands into the primary command-buffer using the standard-method
            currentFrameData->primaryCmd->begin(cmdUsage);
            {
                VkCommandBuffer cmd = currentFrameData->primaryCmd->get();

                // Update dynamic viewport+scissor state
                Framebuffer* fbo = frameResources[frameDataIndex].forwardFramebuffer;
                currentFrameData->primaryCmd->setViewport(fbo);
                currentFrameData->primaryCmd->setScissor(fbo);

                // Draw the scene only with a single pipeline (for wireframe/solid rendering-mode)
                baseShader->bind(cmd);

                // Begin renderpass
                clearRenderpass->begin(cmd, frameResources[frameDataIndex].forwardFramebuffer);

                // Bind View-Projection Set once used by all following Pipelines
                camera->bind(cmd, solidShader->getPipelineLayout());

                // Render all objects
                camera->render(cmd, baseShader, SceneManager::getCurrentScene()->getRenderables(), settings.cull);

                // Draw a sphere around all lights
                auto sphereMesh = MESH(MODEL_SPHERE_DEFAULT);
                sphereMesh->bind(cmd);
                for (const auto& light : SceneManager::getCurrentScene()->getLights())
                {
                    // Update model-matrix through push-constant
                    wireframeShader->pushConstant(cmd, 0, sizeof(Mat4f), &light->getWorldMatrix());

                    // Draw sphere scaled with range of the light
                    sphereMesh->draw(cmd);
                }

                clearRenderpass->end(cmd);
            }
            currentFrameData->primaryCmd->end();
        }
    }

    // Record deferred commands into the given cmd
    void RenderingEngine::recordDeferredLightingCommands(VkCommandBuffer cmd)
    {
        // Framebuffer with a color attachment, which will be loaded.
        loadRenderpassNoDepth->begin(cmd, frameResources[frameDataIndex].lightAccFramebuffer);
        {
            // Bind Camera-Descriptor-Set
            camera->bind(cmd, dirLightShader->getPipelineLayout());

            // Bind descriptor-set referencing the G-Buffer
            gBuffer->bind(cmd, dirLightShader->getPipelineLayout());

            // Render directional-lights
            dirLightShader->bind(cmd);
            camera->render(cmd, dirLightShader, SceneManager::getCurrentScene()->getDirectionalLights());

            // Render point-lights
            pointLightShader->bind(cmd);
            camera->render(cmd, pointLightShader, SceneManager::getCurrentScene()->getPointLights());

            // Render spot-lights
            spotLightShader->bind(cmd);
            camera->render(cmd, spotLightShader, SceneManager::getCurrentScene()->getSpotLights());
        }
        loadRenderpassNoDepth->end(cmd);
    }

    // Record forward-rendering commands into the given cmd
    void RenderingEngine::recordForwardCommands(VkCommandBuffer cmd)
    {
        // Begin renderpass (loads color + depth-buffer instead of clearing it)
        loadRenderpass->begin(cmd, frameResources[frameDataIndex].forwardFramebuffer);
        {
            // Bind camera Set only once. (Always Set-Number 0)
            camera->bind(cmd, gBufferShader->getPipelineLayout());

            for (auto& shader : GET_FORWARD_SHADERS)
            {
                if(!shader->isActive())
                    continue;

                // Bind shader pipe + sets
                shader->bind(cmd);

                for (const auto& mat : shader->getMaterialsFromCurrentScene())
                {
                    mat->bind(cmd);
                    for (const auto& obj : mat->getRenderables())
                        camera->render(cmd, shader, obj, settings.cull);
                }
            }
        }
        loadRenderpass->end(cmd);
    }

    // Transfer the rendered result into an host visible buffer, retrieve it and call the callback
    void RenderingEngine::getRenderedDataAndCallCallback(VulkanImage& renderedImage)
    {
        uint32_t width  = getFinalWidth();
        uint32_t height = getFinalHeight();
        uint32_t bpp    = vkTools::getBytesPerPixel(renderedImage.getFormat());
        uint32_t size   = width * height * bpp;

        // Buffer for the image data
        VulkanBuffer buffer(device0, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        auto cmd = commandPool->allocate();

        // Copy last post-processed image into a buffer, from which i receive the pixel data
        cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        {
            // Transition the layout so we can copy from it
            cmd->setImageLayout(renderedImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            // Copy the image data into the buffer
            cmd->copyImageToBuffer(renderedImage, buffer);

            // Retransition the layout back to shader read
            cmd->setImageLayout(renderedImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        cmd->endSubmitAndWaitForFence(device0, graphicQueue);

        // Retrieve the pixel-data from the buffer
        ImageData imageData;
        imageData.resolution    = Vec2ui(width, height);
        imageData.bytesPerPixel = bpp;
        imageData.pixels.resize(size);
        buffer.copyFrom(imageData.pixels.data(), size);

        // Call callback
        renderingFinishedCallback(imageData);

        // Reset callback
        renderingFinishedCallback = nullptr;
    }

    //---------------------------------------------------------------------------
    //  OnSizeChanged Callback
    //---------------------------------------------------------------------------

    // Called from the window class as a callback if e.g. the window size has changed
    void RenderingEngine::onSizeChanged()
    {
        // Recreates the renderpass
        VulkanBase::onSizeChanged();

        float newWidthFloat = (float)VulkanBase::getFinalWidth();
        float newHeightFloat = (float)VulkanBase::getFinalHeight();

        // Change the aspec ratio from the camera
        if(camera) camera->setAspecRatio(newWidthFloat / newHeightFloat);

        // Notify Sub-Renderer
        for(auto& sr : subRenderer)
            sr.second->onSizeChanged(newWidthFloat, newHeightFloat);
    }

    //---------------------------------------------------------------------------
    //  PreProcess Pipeline
    //---------------------------------------------------------------------------

    void RenderingEngine::createPreProcessBuffer(const Vec2ui& size, VkFormat imageFormat)
    {
        if (preProcessingEnabled) return;
        preProcessingEnabled = true;

        ShaderParams params("PreProcessShader", "/shaders/pre_process", PipelineType::PostProcess, mrtRenderpass);
        preProcessShader = SHADER(params);

        auto sampler = std::make_shared<Sampler>(1.0f, FILTER_LINEAR, FILTER_LINEAR, MIPMAP_MODE_NEAREST);
        preProcessTexture = ADD_RAW_TEXTURE(new Texture(size, imageFormat, sampler ));
        preProcessShader->setTexture("Texture", preProcessTexture);
    }

    void RenderingEngine::fillPreprocessBuffer(void* pixelData)
    {
        if (!preProcessingEnabled) Logger::Log("PreProcessing-Pipeline is disabled. Please call 'createPreProcessBuffer()' first", LOGTYPE_ERROR);
        preProcessTexture->push(pixelData);
    }

    //---------------------------------------------------------------------------
    //  FOG
    //---------------------------------------------------------------------------

    void RenderingEngine::setFOGIsActive(bool isActive)
    {
        auto fogShader = SHADER("FOG");
        fogShader->setActive(isActive);
        if (SHADER_EXISTS("Skybox"))
            SHADER("Skybox")->setFloat("fogDensity", isActive ? 1.0f : 0.0f);
    }

    // Toggle FOG + Skybox FOG
    void RenderingEngine::toggleFOG()
    {
        auto fogShader = SHADER("FOG");
        fogShader->toggleActive();
        if (SHADER_EXISTS("Skybox"))
            SHADER("Skybox")->setFloat("fogDensity", fogShader->isActive() ? 1.0f : 0.0f);
    }

    // Changes the FOG-Color from the fog-post-processing shader AND the skybox-shader if existent
    void RenderingEngine::setFogColor(const Color& fogColor)
    {
        SHADER("FOG")->setColor("fogColor", fogColor);
        if (SHADER_EXISTS("Skybox"))
            SHADER("Skybox")->setColor("fogColor", fogColor);
    }

    Color RenderingEngine::getFogColor() { return SHADER("FOG")->getColor("fogColor"); }
    void RenderingEngine::setFOGDensity(float density) { SHADER("FOG")->setFloat("density", density); }
    float RenderingEngine::getFOGDensity() { return SHADER("FOG")->getFloat("density"); }
    void RenderingEngine::setFOGGradient(float gradient) { SHADER("FOG")->setFloat("gradient", gradient); }
    float RenderingEngine::getFOGGradient() { return SHADER("FOG")->getFloat("gradient"); }

    //---------------------------------------------------------------------------
    //  Other
    //---------------------------------------------------------------------------

    void RenderingEngine::toggleRenderAlbedo()
    {
        int val = gBufferShader->getInt("renderAlbedo");
        if (val == 0) setRenderingMode(ERenderingMode::UNLIT);
        gBufferShader->setInt("renderAlbedo", val == 0 ? 1 : 0);
    }

    void RenderingEngine::toggleRenderNormals()
    {
        int val = gBufferShader->getInt("renderNormals");
        if (val == 0) setRenderingMode(ERenderingMode::UNLIT);
        gBufferShader->setInt("renderNormals", val == 0 ? 1 : 0);
    }

    void RenderingEngine::toggleNormalMapRendering()
    {
        int val = gBufferShader->getInt("renderNormalMaps");
        if (val == 0) setRenderingMode(ERenderingMode::UNLIT);
        gBufferShader->setInt("renderNormalMaps", val == 0 ? 1 : 0);
    }

    void RenderingEngine::toggleDispMapRendering()
    {
        int val = gBufferShader->getInt("renderDispMaps");
        if (val == 0) setRenderingMode(ERenderingMode::UNLIT);
        gBufferShader->setInt("renderDispMaps", val == 0 ? 1 : 0);
    }

    void RenderingEngine::toggleShadows()
    {
        settings.renderShadows = !settings.renderShadows;
        for (const auto& light : SceneManager::getCurrentScene()->getLights())
            light->setShadows(settings.renderShadows);
        subRenderer[SHADOW]->setActive(settings.renderShadows);
    }

    void RenderingEngine::toggleGUI()
    {
        settings.renderGUI = !settings.renderGUI;
        subRenderer[GUI]->setActive(settings.renderGUI);
        window->setMouseCursorVisibiblity(settings.renderGUI);
    }

    void RenderingEngine::setClearColor(const Color& clearColor)
    {
        // Set clear-color for the light accumulation target (which is the first one)
        mrtRenderpass->setColorClearValue(0, clearColor);
    }

    void RenderingEngine::setRenderingMode(ERenderingMode renderingMode)
    {
        this->renderingMode = renderingMode;
        switch (this->renderingMode)
        {
        case ERenderingMode::SOLID:
            baseShader = solidShader;
            break;
        case ERenderingMode::WIREFRAME:
            baseShader = wireframeShader;
            break;
        }
    }

    void RenderingEngine::setRenderCallback(std::function<void(const ImageData&)> func)
    {
        renderingFinishedCallback = func;
    }

    void RenderingEngine::setRenderBoundingBoxes(bool b)
    {
        for (auto& r : MATERIAL_GET(MATERIAL_BOUNDING_BOX)->getRenderablesFromCurrentScene())
            r->setIsActive(b);
    }

    void RenderingEngine::toggleBoundingBoxes()
    {
        static bool showBBs = true;
        setRenderBoundingBoxes(showBBs);
        showBBs = !showBBs;
    }

}