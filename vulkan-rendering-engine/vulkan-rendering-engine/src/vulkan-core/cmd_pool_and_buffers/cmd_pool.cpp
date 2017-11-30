#include "cmd_pool.h"

#include <assert.h>

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------

    CommandPool::CommandPool(VkDevice _device, const uint32_t& queueFamilyIndex)
        : device(_device)
    {
        VkCommandPoolCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = queueFamilyIndex;

        VkResult res = vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
        assert(res == VK_SUCCESS);
    }

    CommandPool::CommandPool(VkDevice _device, const uint32_t& queueFamilyIndex, const VkCommandPoolCreateFlags& flags)
        : device(_device)
    {
        VkCommandPoolCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = flags;
        createInfo.queueFamilyIndex = queueFamilyIndex;

        VkResult res = vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
        assert(res == VK_SUCCESS);
    }


    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    SCommandBuffer CommandPool::allocate(VkCommandBufferLevel level)
    {
        SCommandBuffer commandBuffer = SCommandBuffer(new CommandBuffer(this));

        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext              = nullptr;
        allocateInfo.commandPool        = commandPool;
        allocateInfo.level              = level;
        allocateInfo.commandBufferCount = 1;

        VkResult res = vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer->cmd);
        assert(res == VK_SUCCESS);

        return std::move(commandBuffer);
    }

    std::vector<SCommandBuffer> CommandPool::allocate(uint32_t num, VkCommandBufferLevel level)
    {
        VkCommandBuffer* pCommandBuffers = new VkCommandBuffer[num];

        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext              = nullptr;
        allocateInfo.commandPool        = commandPool;
        allocateInfo.level              = level;
        allocateInfo.commandBufferCount = num;

        VkResult res = vkAllocateCommandBuffers(device, &allocateInfo, pCommandBuffers);
        assert(res == VK_SUCCESS);

        std::vector<SCommandBuffer> commandBuffers;
        for (uint32_t i = 0; i < num; i++)
        {
            auto cmd = SCommandBuffer(new CommandBuffer(this));
            cmd->cmd = pCommandBuffers[i];
            commandBuffers.push_back(std::move(cmd));
        }
        delete pCommandBuffers;

        return std::move(commandBuffers);
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Free memory from the given command buffer
    void CommandPool::deallocate(CommandBuffer* commandBuffer)
    {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer->cmd);
    }


}