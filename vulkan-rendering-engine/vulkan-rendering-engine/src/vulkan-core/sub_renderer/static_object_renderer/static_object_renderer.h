#ifndef STATIC_OBJECT_RENDERER_H_
#define STATIC_OBJECT_RENDERER_H_

#include "../sub_renderer.h"

namespace ENGINE
{

    //---------------------------------------------------------------------------
    //  StaticObjectRenderer - Class
    //---------------------------------------------------------------------------

    class StaticObjectRenderer : public SubRenderer
    {
    public:
        StaticObjectRenderer(RenderingEngine* renderingEngine);
        ~StaticObjectRenderer() {}

        // Return a secondary-cmd for the given frame-resource index. Rerecord if necessary.
        VkCommandBuffer getSecCmd(uint32_t index) override;

        // Called from the Rendering-Engine when the window size has changed.
        void onSizeChanged() override;

    private:
        // Record the draw-commands into the command buffers
        void preRecordCommandBuffers();

    };


}

#endif // !STATIC_OBJECT_RENDERER_H_
