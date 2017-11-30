#include "vulkan_buffer.h"

#include "vulkan-core/memory_management/vulkan_memory_manager.h"
#include <assert.h>

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------


    VulkanBuffer::VulkanBuffer(VkDevice _device, const VkDeviceSize& _size, const VkBufferUsageFlags& usage, const VkFlags& requirementsMask)
        : device(_device), size(_size)
    {
        if((requirementsMask & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
            canBeMapped = true;

        createBuffer(usage, requirementsMask);
    }
    
    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    VulkanBuffer::~VulkanBuffer()
    {
        if(isMapped)
            unmap();

        vkDeviceWaitIdle(device);
        vkDestroyBuffer(device, buffer, nullptr);
        VMM::freeMemory(memory);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void* VulkanBuffer::map(const VkDeviceSize& offset, const VkDeviceSize& mapSize)
    {
        assert(canBeMapped);
        isMapped = true;

        void* pData;
        vkMapMemory(device, memory, offset, mapSize, 0, (void **)&pData);
        return pData;
    }

    void VulkanBuffer::unmap()
    {
        assert(canBeMapped);
        isMapped = false;

        vkUnmapMemory(device, memory);
    }

    void VulkanBuffer::copyInto(const void* data, const std::size_t& copySize, const VkDeviceSize& offset)
    {
        assert((offset + copySize) <= size);
        void* mappedPtr = map(offset);
        memcpy(mappedPtr, data, copySize == WHOLE_BUFFER_SIZE ? (std::size_t)size : copySize);
        unmap();
    }

    void VulkanBuffer::copyFrom(void* data, const std::size_t& copySize, const VkDeviceSize& offset)
    {
        assert((offset + copySize) <= size);
        void* mappedPtr = map(offset);
        memcpy(data, mappedPtr, copySize == WHOLE_BUFFER_SIZE ? (std::size_t)size : copySize);
        unmap();
    }

    // Constructs a buffer-info struct for this buffer and return it
    VkDescriptorBufferInfo VulkanBuffer::getDescriptorBufferInfo(const VkDeviceSize& offset, const VkDeviceSize& range)
    {
        VkDescriptorBufferInfo result;
        result.buffer = buffer;
        result.offset = offset;
        result.range  = range;

        return result;
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    void VulkanBuffer::createBuffer(const VkBufferUsageFlags& usage, const VkFlags& requirementsMask)
    {
        VkResult err;

        VkBufferCreateInfo bufferInfo;
        bufferInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext                 = nullptr;
        bufferInfo.flags                 = 0;
        bufferInfo.size                  = size;
        bufferInfo.usage                 = usage;
        bufferInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 0;
        bufferInfo.pQueueFamilyIndices   = nullptr;

        err = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
        assert(!err);

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device, buffer, &memReqs);

        memory = VMM::allocateMemory(memReqs, requirementsMask);

        err = vkBindBufferMemory(device, buffer, memory, 0);
        assert(!err);
    }



    //---------------------------------------------------------------------------
    //  VertexBuffer class
    //---------------------------------------------------------------------------

    void VulkanVertexBuffer::bind(VkCommandBuffer cmd, const uint32_t& firstBinding)
    {
        // Bind buffer
        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(cmd, firstBinding, 1, &buffer, offsets);
    }

    //---------------------------------------------------------------------------
    //  IndexBuffer class
    //---------------------------------------------------------------------------

    void VulkanIndexBuffer::bind(VkCommandBuffer cmd, const VkDeviceSize& offset, const VkIndexType& type)
    {
        // Bind buffer
        vkCmdBindIndexBuffer(cmd, buffer, offset, type);
    }


}