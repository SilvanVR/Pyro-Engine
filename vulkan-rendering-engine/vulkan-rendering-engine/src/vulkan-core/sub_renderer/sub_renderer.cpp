#include "sub_renderer.h"

#include "vulkan-core/rendering_engine.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    SubRenderer::SubRenderer(RenderingEngine* r, bool primaryCmds)
        : renderingEngine(r), device(r->device0), frameResources(r->frameResources), frameDataIndex(r->frameDataIndex)
    {
        // Create command-pool
        commandPool = std::unique_ptr<CommandPool>(new CommandPool(device, r->deviceManager.getQueueFamilyGraphicsIndex()));

        // Cmd-Buffers
        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        if (!primaryCmds)
            level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

        cmdBuffers = commandPool->allocate(static_cast<uint32_t>(frameResources.size()), level);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    SubRenderer::~SubRenderer()
    { 
 
    }



}