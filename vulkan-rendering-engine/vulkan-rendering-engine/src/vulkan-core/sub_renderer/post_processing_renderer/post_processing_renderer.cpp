#include "post_processing_renderer.h"

#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/pipelines/renderpass/renderpass.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "vulkan-core/rendering_engine.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    PostProcessingRenderer* PostProcessingRenderer::INSTANCE;

    void PostProcessingRenderer::addPostProcess(PostProcessStep* postProcess)
    { 
        INSTANCE->postProcessSteps.push_back(postProcess); 
    }

    void PostProcessingRenderer::removePostProcess(PostProcessStep* postProcess)
    { 
        removeObjectFromList(INSTANCE->postProcessSteps, postProcess);
    }

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    PostProcessingRenderer::PostProcessingRenderer(RenderingEngine* r)
        : SubRenderer(r, true)
    {
        if(INSTANCE == nullptr)
            INSTANCE = this;
        else
            Logger::Log("PostProcessingRenderer::PostProcessingRenderer(): Two Objects were created, but only one is allowed.", LOGTYPE_ERROR);

        // ORDER IMPORTANT!!
        setupRenderpass(r->getSurfaceFormat());
        sampler = new VulkanSampler(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

        // Create the "empty" post-processing effect
        noneStep = new SinglePostProcessStep(renderpass, "None", "/shaders/post_process/none");

        // Add all Post-Process-Steps here
        postProcessSteps.push_back(new HDRBloom(hdrRenderpass, 1.0f, 0.5f, 1.0f));
        SHADER("HDRBloom")->toggleActive();

        // Fog needs to calculated before FXAA and Tonemapping
        postProcessSteps.push_back(new FOG(hdrRenderpass, "FOG", 0.1f, 3000.0f));
        SHADER("FOG")->toggleActive();

        // Every renderpass BEFORE TONEMAP MUST BE "hdrRenderpass" to PRESERVE the 16-BIT FLOAT DATA !!!
        postProcessSteps.push_back(new Tonemap(renderpass, "Tonemap", 2.0f, 2.2f));
        postProcessSteps.push_back(new FXAA(renderpass, "FXAA"));
        //postProcessSteps.push_back(new ContrastChanger(renderpass, nearestSampler, "Contrast", 0.1f));
        
        postProcessSteps.push_back(new DepthOfField(renderpass, 50.0f, 30.0f));
        SHADER("DepthOfField")->toggleActive();

        postProcessSteps.push_back(new CameraDirtTexture(renderpass, "CameraDirt", 5.0f));
        SHADER("CameraDirt")->toggleActive();

        lensFlareRenderer = new LensFlareRenderer(loadRenderpass, "LensFlares");
        postProcessSteps.push_back(lensFlareRenderer);
        lensFlareRenderer->toggleActive();

        lightShafts = new LightShafts(renderpass, renderpassClearColorLoadDepth);
        postProcessSteps.push_back(lightShafts);
        lightShafts->toggleActive();

        //postProcessSteps.push_back(new SinglePostProcessStep(renderpass, nearestSampler, "test", "/shaders/post_process/hdr_bloom_filter"));
        //postProcessSteps.push_back(new RadialBlur(renderpass, sampler, "RadialBlur", 0.35f, 1.0f, Vec2f(0.5f, 0.5f), 0.5f));
        //postProcessSteps.push_back(new GaussianBlur9x1(renderpass, sampler, "HBlur1", true, 1.0f, 1.0f , 0.5f));
        //postProcessSteps.push_back(new GaussianBlur9x1(renderpass, sampler, "VBlur1", false, 1.0f, 1.0f, 0.5f));
        //postProcessSteps.push_back(new SinglePostProcessStep(renderpass, nearestSampler, "SunFilter", "/shaders/post_process/sun_filter"));
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    PostProcessingRenderer::~PostProcessingRenderer()
    {
        for(auto& postProcess : postProcessSteps)
            delete postProcess;
        delete renderpass;
        delete hdrRenderpass;
        delete loadRenderpass;
        delete renderpassClearColorLoadDepth;
        delete noneStep;
        delete sampler;
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void PostProcessingRenderer::update(float delta)
    {
        for (auto& postProcess : postProcessSteps)
            if(postProcess->isActive())
                postProcess->update(delta);

        // Light-Shafts needs always to be updated, because it calculates the sun's position on screen
        if(!lightShafts->isActive()) 
            lightShafts->update(delta);
    }

    // Record commands from all Post-Processing-Steps into a cmd in this class
    void PostProcessingRenderer::recordCommandBuffer(uint32_t frameDataIndex, Framebuffer* sceneFramebuffer)
    {
        cmdBuffers[frameDataIndex]->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        {
            uint32_t postProcessStepCount = 0;

            // Do all Post-Processing Step-By-Step. First input is the normal scene-rendering framebuffer.
            lastFramebuffer = sceneFramebuffer;

            if(VulkanBase::getSettings().doPostProcessing)
            {
                for (auto& postProcessStep : postProcessSteps)
                {
                    if(!postProcessStep->isActive())
                        continue;

                    postProcessStepCount++;

                    // Record Post-Process commands into the given cmd using the given framebuffer as the input
                    // Some stages might use the original scene-framebuffer, thats why it is passed separately too
                    postProcessStep->record(cmdBuffers[frameDataIndex].get(), { lastFramebuffer }, sceneFramebuffer);

                    // Next Post-Process step should use the previous framebuffer as input
                    lastFramebuffer = postProcessStep->getOutputFramebuffer();
                }
            }
            
            // If we did zero post-process steps or the resolution of the last post-processing framebuffer is lower than the screen,
            // we have to do this to scale the scene-framebuffer up to the Window - resolution
            if (postProcessStepCount == 0 || (lastFramebuffer->getWidth() != VulkanBase::getFinalWidth() && lastFramebuffer->getHeight() != VulkanBase::getFinalHeight()))
            {
                noneStep->record(cmdBuffers[frameDataIndex].get(), { lastFramebuffer }, sceneFramebuffer);
                lastFramebuffer = noneStep->getOutputFramebuffer();
            }
        }
        cmdBuffers[frameDataIndex]->end();
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Create a renderpass with only a color-attachment
    // All Post-Processing-Steps will use this for rendering
    void PostProcessingRenderer::setupRenderpass(const VkFormat& colorFormat)
    {
        // Color attachment
        VkAttachmentDescription colorAttachment, depthAttachment;
        colorAttachment.flags           = 0;
        colorAttachment.format          = colorFormat;
        colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Renderpass for basic post-processing
        renderpass = new Renderpass(device, { colorAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } );

        // Renderpass for lens-flares
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        loadRenderpass = new Renderpass(device, { colorAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

        // Depth attachment
        depthAttachment.flags           = 0;
        depthAttachment.format          = GBUFFER_DEPTH_FORMAT;
        depthAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        depthAttachment.finalLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // IMPORTANT ON AMD - GPU's!!!!

        renderpassClearColorLoadDepth = new Renderpass(device, { colorAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
                                                      { depthAttachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });

        // Renderpass for framebuffers with a floating-point attachment
        colorAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        hdrRenderpass = new Renderpass(device, { colorAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
    }

    //---------------------------------------------------------------------------
    //  OnSizeChanged - Callback
    //---------------------------------------------------------------------------

    // Called from the Rendering-Engine when the window size has changed.
    void PostProcessingRenderer::onSizeChanged(float newWidth, float newHeight)
    {
        for(auto& step : postProcessSteps)
            step->onSizeChanged(newWidth, newHeight);
        noneStep->onSizeChanged(newWidth, newHeight);
    }

}