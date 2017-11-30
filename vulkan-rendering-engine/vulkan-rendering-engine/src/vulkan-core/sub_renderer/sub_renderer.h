#ifndef SUB_RENDERER_H_
#define SUB_RENDERER_H_

#include "build_options.h"
#include "vulkan-core/vulkan_base.h"

namespace Pyro
{

    class RenderingEngine;

    //---------------------------------------------------------------------------
    //  SubRenderer - Class
    //---------------------------------------------------------------------------

    class SubRenderer
    {
    public:
        SubRenderer(RenderingEngine* r, bool primaryCmds = true);
        virtual ~SubRenderer();

    public:
        // Called from the Rendering-Engine when the window size has changed.
        virtual void            onSizeChanged(float newWidth, float newHeight) {}
        virtual void            update(float delta) {}

        // Return a command-buffer from the appropriate frame-resource index.
        CommandBuffer*          getCMD(uint32_t frameDataIndex) { return cmdBuffers[frameDataIndex].get(); }

        // Enable / Disable this renderer
        void                    toggleActive(){ m_isActive = !m_isActive; }
        bool                    isActive(){ return m_isActive; }
        void                    setActive(bool b){ m_isActive = b; }

        // Some Virtual-Functions. SubRenderer can override them if needed.
        virtual void            recordCommandBuffer(uint32_t frameDataIndex) {}
        virtual void            recordCommandBuffer(uint32_t frameDataIndex, Framebuffer* framebuffer) {}
        virtual Framebuffer*    getOutputFramebuffer() { return nullptr; }

    protected:
        VkDevice                     device;             // Reference to the logical device
        RenderingEngine*             renderingEngine;    // Reference to the renderingEngine itself
                                     
        std::unique_ptr<CommandPool> commandPool;        // Command-pool for the cmdBuffers. A new pool allows this renderer to be used by a different thread.
        std::vector<SCommandBuffer>  cmdBuffers;         // cmd-buffers used for rendering

        std::vector<FrameData>&      frameResources;     // Frame-Resources reference from the RenderingEngine
        uint32_t&                    frameDataIndex;     // Current FrameDataIndex

        bool                         m_isActive = true;  // True: This subrenderer is active
    };

}

#endif // !SUB_RENDERER_H_
