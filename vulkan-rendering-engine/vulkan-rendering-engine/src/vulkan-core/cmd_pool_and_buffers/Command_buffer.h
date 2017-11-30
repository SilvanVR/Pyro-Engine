/*
*  ComandPool-Class header file.
*  Encapsulates a VkCommandBuffer. Construct a Command buffer without backing memory.
*  Before using make sure to allocate memory from a CommandPool-Class.
*
*  Holds a static Command Buffer used by the swapchain for layout transitions (if the window size has changed)
*
*  Date:    25.04.2016
*  Creator: Silvan Hau
*/

#ifndef COMMAND_BUFFER_H_
#define COMMAND_BUFFER_H_

#include "build_options.h"
#include "vulkan-core/util_classes/vulkan_buffer.h"
#include "vulkan-core/util_classes/vulkan_image.h"
#include "vulkan-core/util_classes/vulkan_other.h"

namespace Pyro
{

    class Framebuffer;
    class CommandPool;

    //---------------------------------------------------------------------------
    //  CommandBuffer class
    //---------------------------------------------------------------------------

    using SCommandBuffer = std::shared_ptr<CommandBuffer>;

    class CommandBuffer
    {
        friend class CommandPool; // Allow only the CommandPool to create CommandBuffer-Objects

        CommandBuffer(CommandPool* commandPool);

    public:
        ~CommandBuffer();

        // Return the VkCommandBuffer
        const VkCommandBuffer& get() const { return cmd; }

        // Some useful functions..
        void begin(const VkCommandBufferUsageFlags& usage, const VkCommandBufferInheritanceInfo* inheritanceInfo = nullptr);
        void end();
        void reset();
        void submit(VkQueue queue, const VulkanFence* fence = nullptr);
        void submit(VkQueue queue, const VkPipelineStageFlags& waitDstStageMask,
                    const VulkanSemaphore* waitSemaphore, const VulkanSemaphore* signalSemaphore,
                    const VulkanFence* fence = nullptr);
        void submit(VkQueue queue, const std::vector<VkPipelineStageFlags>& waitDstStageMask,
                    const std::vector<const VulkanSemaphore*>& waitSemaphore, const VulkanSemaphore* signalSemaphore,
                    const VulkanFence* fence = nullptr);

        // End the Command-Buffer. Submit it and wait on a Fence for completion. (This function returns when all commands in the cmd has finished)
        void endSubmitAndWaitForFence(VkDevice device, VkQueue queue);

        // Put a command in this CommandBuffer: Copy an buffer with "vkCmdCopyBuffer"
        void copyBuffer(const VulkanBuffer& srcBuffer, const VulkanBuffer& dstBuffer, const VkDeviceSize& size);

        // Put a command in this CommandBuffer: Copy a buffer to the given VkImage with "vkCmdCopyBufferToImage"
        void copyBufferToImage(const VulkanBuffer& srcBuffer, const VulkanImage& dstImage);
        void copyBufferToImage(const VulkanBuffer& srcBuffer, const VulkanImage& dstImage, const std::vector<VkBufferImageCopy>& pRegions);

        // Put a command in this CommandBuffer: Copy a image to the given buffer with "vkCmdCopyImageToBuffer"
        void copyImageToBuffer(const VulkanImage& srcImage, const VulkanBuffer& dstBuffer);
        void copyImageToBuffer(const VulkanImage& srcImage, const VulkanBuffer& dstBuffer, const std::vector<VkBufferImageCopy>& pRegions);

        // Put a command in this CommandBuffer: Copy an image with "vkCmdCopyImage"
        void copyImage(const VulkanImage& srcImage, const VulkanImage& dstImage, uint32_t baseArrayLayer = 0, uint32_t mipLevel = 0);

        // Change the layout of an image using a Pipeline-Barrier
        void setImageLayout(VulkanImage& image, const VkImageLayout& newLayout);
        void setImageLayout(VulkanImage& image, const VkImageLayout& newLayout, uint32_t baseMipLevel);
        void setImageLayout(VulkanImage& image, const VkImageLayout& newLayout, const VkImageSubresourceRange& subResource);

        // Set dynamic viewport. Use whole fbo-size.
        void setViewport(Framebuffer* fbo);

        // Set dynamic scissor. Use whole fbo-size.
        void setScissor(Framebuffer* fbo);

        // Put a pipeline-barrier into this cmd
        void pipelineBarrier(const VkPipelineStageFlags& srcStages, const VkPipelineStageFlags& dstStages,
                             const VkAccessFlags& srcAccessMask, const VkAccessFlags& dstAccessMask);

        // Push-Constants
        void pushConstants(const VkPipelineLayout& pipeLayout, const VkShaderStageFlags& shaderStage,
                           const uint32_t& offset, const uint32_t& size, const void* pValues);

        //---------------------------------------------------------------------------
        //  Static Methods
        //---------------------------------------------------------------------------

        // Submit a bunch of command buffers to the given queue
        static void submit(VkQueue queue, const std::vector<const CommandBuffer*>& commandBuffers, const VulkanFence* fence = nullptr);

        // Submit a bunch of command buffers to the given queue
        static void submit(VkQueue queue, const std::vector<const CommandBuffer*>& commandBuffers,
                           const std::vector<const VulkanSemaphore*>& waitSemaphores, const std::vector<VkPipelineStageFlags>& waitDstStageMasks,
                           const std::vector<const VulkanSemaphore*>& signalSemaphores, const VulkanFence* fence);

    private:
        // forbid assignment
        CommandBuffer& operator=(const CommandBuffer& cmd) {};

        // The VkCommandBuffer
        VkCommandBuffer cmd;

        // The pool this CommandBuffer was allocated from
        CommandPool* commandPool;


        // Put a vkPipelineBarrier into the given Command Buffer
        void setImageLayout(const VkImage& image, const VkImageAspectFlags& aspectMask, const VkImageLayout& oldLayout, const VkImageLayout& newLayout);

        void setImageLayout(const VkImage& image, const VkImageLayout& oldLayout, const VkImageLayout& newLayout, const VkImageSubresourceRange& subResource);

        void setImageLayout(const VkImage& image, const VkImageAspectFlags& aspectMask, const VkImageLayout& oldLayout, const VkImageLayout& newLayout,
            const VkAccessFlags& srcAccessMask, const VkAccessFlags& dstAccessMask);

        void setImageLayout(const VkImage& image, const VkImageAspectFlags& aspectMask, const VkImageLayout& oldLayout, const VkImageLayout& newLayout,
            const VkAccessFlags& srcAccessMask, const VkAccessFlags& dstAccessMask,
            const VkPipelineStageFlags& srcStages, const VkPipelineStageFlags& destStages);

        void setImageLayout(const VkImage& image, const VkImageAspectFlags& aspectMask, const VkImageLayout& oldLayout, const VkImageLayout& newLayout,
            const VkAccessFlags& srcAccessMask, const VkAccessFlags& dstAccessMask,
            const uint32_t& srcQueueFamily, const uint32_t& dstQueueFamily,
            const VkPipelineStageFlags& srcStages, const VkPipelineStageFlags& destStages);

        void setImageLayout(const VkImage& image, const VkImageLayout& oldLayout, const VkImageLayout& newLayout,
            const VkAccessFlags& srcAccessMask, const VkAccessFlags& dstAccessMask,
            const uint32_t& srcQueueFamily, const uint32_t& dstQueueFamily,
            const VkPipelineStageFlags& srcStages, const VkPipelineStageFlags& destStages, const VkImageSubresourceRange& subResource);
    };

}


#endif // !COMMAND_BUFFER_H_
