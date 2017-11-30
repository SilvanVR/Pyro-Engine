/*
*  ComandPool-Class header file.
*  Encapsulates a VkCommandPool and allow easy Command-Buffer allocations.
*
*  Date:    25.04.2016
*  Creator: Silvan Hau
*/

#ifndef CMD_MANAGER_H_
#define CMD_MANAGER_H_

#include "build_options.h"
#include "command_buffer.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  CmdManager class
    //---------------------------------------------------------------------------

    class CommandPool
    {
        friend class CommandBuffer; //Allow this Class to access "allocate"

    public:
        // Create a VkCommandPool compatible with the given queueFamilyIndex
        CommandPool(VkDevice device, const uint32_t& queueFamilyIndex);
        // Create a VkCommandPool compatible with the given queueFamilyIndex
        CommandPool(VkDevice device, const uint32_t& queueFamilyIndex, const VkCommandPoolCreateFlags& flags);

        // Clean up the VkCommandPool
        ~CommandPool() { vkDestroyCommandPool(device, commandPool, nullptr); };

        // Return the VkCommandPool
        VkCommandPool get() { return commandPool; }

        // Allocate one command buffer from this pool
        SCommandBuffer allocate(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        // Allocate several command buffer from this pool
        std::vector<SCommandBuffer> allocate(uint32_t num, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    private:
        // forbid copy and copy assignment
        CommandPool(const CommandPool& commandPool);
        CommandPool& operator=(const CommandPool& commandPool) {};

        // The VkCommandPool handle
        VkCommandPool commandPool;

        // Need this as a reference for destuction of the VkCommandPool
        VkDevice device;

        // Free memory from the given command buffer. Called in the CommandBuffers-Deconstructor
        void deallocate(CommandBuffer* commandBuffer);

    };


}



#endif // !CMD_MANAGER_H_
