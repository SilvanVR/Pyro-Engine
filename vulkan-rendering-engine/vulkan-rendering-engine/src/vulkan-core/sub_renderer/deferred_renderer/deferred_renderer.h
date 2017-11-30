#ifndef _DEFERRED_RENDERER_H_
#define _DEFERRED_RENDDERER_H_

#include "../sub_renderer.h"


namespace ENGINE
{


    class DeferredRenderer : public SubRenderer
    {
    public:
        DeferredRenderer(RenderingEngine* r);
        ~DeferredRenderer();


        void recordCommandBuffer(uint32_t frameDataIndex, Framebuffer* framebuffer);

        void record(uint32_t frameDataIndex, VkCommandBuffer cmd);

        void recordWired(uint32_t frameDataIndex, VkCommandBuffer cmd);

        // Called from the Rendering-Engine when the window size has changed.
        void onSizeChanged() override;

    private:
        Shader          dirLightShader;     // Self-Explained
        Shader          pointLightShader;   // Self-Explained
        Shader          spotLightShader;    // Self-Explained

        Mesh            sphereMesh;

        Renderpass*&    loadRenderpass;
    };

}


#endif // !_DEFERRED_RENDERER_H_

