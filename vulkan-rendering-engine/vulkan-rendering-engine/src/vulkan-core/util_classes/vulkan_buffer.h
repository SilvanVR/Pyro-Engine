#ifndef VULKAN_BUFFER_H_
#define VULKAN_BUFFER_H_


#include "build_options.h"

namespace Pyro
{
    
    //---------------------------------------------------------------------------
    //  VulkanBuffer class
    //---------------------------------------------------------------------------

    class VulkanBuffer
    {

    public:
        VulkanBuffer(VkDevice device, const VkDeviceSize& size, const VkBufferUsageFlags& usage, const VkFlags& requirementsMask);
        virtual ~VulkanBuffer();

        // Map this buffer and return a pointer to the data. This is only possible when this buffer
        // were created with the VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT bit set.
        void* map(const VkDeviceSize& offset = 0, const VkDeviceSize& size = VK_WHOLE_SIZE);
        void unmap();

        // Constructs a buffer-info struct for this buffer and return it
        VkDescriptorBufferInfo getDescriptorBufferInfo(const VkDeviceSize& offset, const VkDeviceSize& range);

        // Copy the given data into this buffer. This is only possible when this buffer
        // were created with the VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT bit set.
        void copyInto(const void* data, const std::size_t& size = WHOLE_BUFFER_SIZE, const VkDeviceSize& offset = 0);

        // Copy data from the buffer into the given pointer. This is only possible when this buffer
        // were created with the VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT bit set.
        void copyFrom(void* data, const std::size_t& size = WHOLE_BUFFER_SIZE, const VkDeviceSize& offset = 0);

        // Getter's
        const VkDeviceSize& getSize() const { return size; }

    protected:
        VkDevice        device;

        VkBuffer        buffer;
        VkDeviceMemory  memory;
        VkDeviceSize    size;

        bool canBeMapped = false;
        bool isMapped = false;

        const VkBuffer& get() const { return buffer; }

        void createBuffer(const VkBufferUsageFlags& usage, const VkFlags& requirementsMask);

        // Allow the Command-Buffer class to access this function
        friend class CommandBuffer;
    };

    //---------------------------------------------------------------------------
    //  VertexBuffer class
    //---------------------------------------------------------------------------

    class VulkanVertexBuffer : public VulkanBuffer
    {

    public:
        VulkanVertexBuffer(VkDevice device, const VkDeviceSize& size, const VkBufferUsageFlags& usage, const VkFlags& requirementsMask)
            : VulkanBuffer(device, size, usage, requirementsMask) {}
        ~VulkanVertexBuffer() {}
        
        // Bind this vertex buffer to the given cmd
        void bind(VkCommandBuffer cmd, const uint32_t& firstBinding);

    };

    //---------------------------------------------------------------------------
    //  IndexBuffer class
    //---------------------------------------------------------------------------

    class VulkanIndexBuffer : public VulkanBuffer
    {

    public:
        VulkanIndexBuffer(VkDevice device, const VkDeviceSize& size, const VkBufferUsageFlags& usage, const VkFlags& requirementsMask)
            : VulkanBuffer(device, size, usage, requirementsMask) {}
        ~VulkanIndexBuffer() {}

        // Bind this index buffer to the given cmd
        void bind(VkCommandBuffer cmd, const VkDeviceSize& offset = 0, const VkIndexType& type = VK_INDEX_TYPE_UINT32);

    };
}


#endif // !VULKAN_BUFFER_H_
