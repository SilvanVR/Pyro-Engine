#ifndef POST_PROCESSING_RENDERER_H_
#define POST_PROCESSING_RENDERER_H_

#include "../sub_renderer.h"
#include "vulkan-core/post_processing/post_processing.h"

// Handles all Post-Processing effects
// Render the scene on a full-screen quad using a unlimited amount of different post-processing shaders

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  PostProcessingRenderer class
    //---------------------------------------------------------------------------

    class PostProcessingRenderer : public SubRenderer
    {
    public:
        PostProcessingRenderer(RenderingEngine* r);
        ~PostProcessingRenderer();

        void update(float delta) override;

        // Record commands from all Post-Processing-Steps into a cmd in this class
        void recordCommandBuffer(uint32_t frameDataIndex, Framebuffer* framebuffer);

        // Return the framebuffer in which the last post-process-step has rendered
        Framebuffer* getOutputFramebuffer() override { return lastFramebuffer; }

        static Renderpass* getRenderpass(){ return INSTANCE->renderpass; }
        static const VulkanSampler* getSampler(){ return INSTANCE->sampler; }
        static LightShafts* getLightShafts(){ return INSTANCE->lightShafts; }
        static LensFlareRenderer* getLensFlareRenderer() { return INSTANCE->lensFlareRenderer; }

        // Add a separate Post-Process to this class. Allows to add post-processes in code rather than here.
        static void addPostProcess(PostProcessStep* postProcess);
        static void removePostProcess(PostProcessStep* postProcess);

        // Called from the Rendering-Engine when the window size has changed.
        void onSizeChanged(float newWidth, float newHeight) override;

    private:
        static PostProcessingRenderer*  INSTANCE;

        std::vector<PostProcessStep*>   postProcessSteps;   // Contains all Post-Processing Steps

        Renderpass*                     loadRenderpass;     // Used for lens-flares
        Renderpass*                     renderpass;         // Renderpass used for all post-process-steps after TONEMAPPING
        Renderpass*                     hdrRenderpass;      // Renderpass used for all post-process-steps before TONEMAPPING
        Renderpass*                     renderpassClearColorLoadDepth; // Renderpass which clears the color but loads the depth attachment
        VulkanSampler*                  sampler;            // Sampler used for sampling the input attachments
        LightShafts*                    lightShafts;        // Object which manages whole lightshaft rendering
        LensFlareRenderer*              lensFlareRenderer;  // Object which manages the whole lens-flare rendering
        Framebuffer*                    lastFramebuffer;    // The last framebuffer in which a post-process-step has rendered

        // We have to do this step, if we have no other post-process effects.
        // This is necessary because we could render the scene into a image with a lower-resolution
        // than the screen. This step effectively scales the image up.
        SinglePostProcessStep*          noneStep;

        // Create a renderpass with only a color-attachment
        // All Post-Processing-Steps will use this for rendering
        void setupRenderpass(const VkFormat& colorFormat);
    };

}




#endif // !POST_PROCESSING_RENDERER_H_

