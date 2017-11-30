#include "command_buffer.h"

#include "cmd_pool.h"
#include "vulkan-core/pipelines/framebuffers/framebuffer.h"
#include "vulkan-core/vkTools/vk_tools.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    CommandBuffer::CommandBuffer(CommandPool* _commandPool)
        : commandPool(_commandPool)
    {
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    CommandBuffer::~CommandBuffer()
    {
        commandPool->deallocate(this);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void CommandBuffer::begin(const VkCommandBufferUsageFlags& usage, const VkCommandBufferInheritanceInfo* inheritanceInfo)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = usage;
        beginInfo.pInheritanceInfo = inheritanceInfo;

        VkResult res = vkBeginCommandBuffer(cmd, &beginInfo);
        assert(res == VK_SUCCESS);
    }

    void CommandBuffer::end()
    {
        VkResult res = vkEndCommandBuffer(cmd);
        assert(res == VK_SUCCESS);
    }

    void CommandBuffer::reset()
    {
        VkResult res = vkResetCommandBuffer(cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        assert(res == VK_SUCCESS);
    }

    void CommandBuffer::submit(VkQueue queue, const VulkanFence* fence)
    {
        submit(queue, NULL, nullptr, nullptr, fence);
    }

    void CommandBuffer::submit(VkQueue queue, const VkPipelineStageFlags& waitDstStageMask,
                               const VulkanSemaphore* waitSemaphore, const VulkanSemaphore* signalSemaphore,
                               const VulkanFence* fence)
    {
        std::vector<const VulkanSemaphore*> semaphore;
        if(waitSemaphore != nullptr)
            semaphore.push_back(waitSemaphore);
        std::vector<VkPipelineStageFlags> waitDstStageMasks;
        if (waitDstStageMask != 0)
            waitDstStageMasks.push_back(waitDstStageMask);
        submit(queue, waitDstStageMasks, semaphore, signalSemaphore, fence);
    }

    void CommandBuffer::submit(VkQueue queue, const std::vector<VkPipelineStageFlags>& waitDstStageMask,
                               const std::vector<const VulkanSemaphore*>& waitSems, const VulkanSemaphore* signalSemaphore,
                               const VulkanFence* fence)
    {
        assert(waitDstStageMask.size() == waitSems.size());

        auto waitSemaphore = getVkList<VkSemaphore, VulkanSemaphore>(waitSems);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext                = nullptr;
        submitInfo.waitSemaphoreCount   = static_cast<uint32_t>(waitSemaphore.size());
        submitInfo.pWaitSemaphores      = waitSemaphore.data();
        submitInfo.pWaitDstStageMask    = waitDstStageMask.data();
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &cmd;
        submitInfo.signalSemaphoreCount = signalSemaphore ? 1 : 0;
        submitInfo.pSignalSemaphores    = signalSemaphore ? &signalSemaphore->get() : nullptr;

        VkResult res = vkQueueSubmit(queue, 1, &submitInfo, fence ? fence->get() : VK_NULL_HANDLE);
        assert(res == VK_SUCCESS);
    }

    // End the Command-Buffer. Submit it and wait on a Fence for completion. (This function returns when all commands in the cmd has finished)
    void CommandBuffer::endSubmitAndWaitForFence(VkDevice device, VkQueue queue)
    {
        // End recording
        this->end();

        VulkanFence fence(device);

        // Submit command buffer and signal the given fence
        this->submit(queue, &fence);

        // Wait for Fence
        fence.wait();
    }

    //---------------------------------------------------------------------------
    //  Public Methods - Buffer Functions
    //---------------------------------------------------------------------------

    // VkCmdFunctions helper functions
    void CommandBuffer::copyBuffer(const VulkanBuffer& srcBuffer, const VulkanBuffer& dstBuffer, const VkDeviceSize& size)
    {
        //assert(isRecording == true);

        VkBufferCopy copyRegion;
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        vkCmdCopyBuffer(cmd, srcBuffer.get(), dstBuffer.get(), 1, &copyRegion);
    }

    void CommandBuffer::copyBufferToImage(const VulkanBuffer& srcBuffer, const VulkanImage& dstImage)
    {
        VkBufferImageCopy bufferCopy = { 0, 0, 0,{ dstImage.getAspectMask(), 0, 0, 1 }, {}, { dstImage.getWidth(), dstImage.getHeight(), 1 } };
        vkCmdCopyBufferToImage(cmd, srcBuffer.get(), dstImage.get(), dstImage.getLayout(), 1, &bufferCopy);
    }

    void CommandBuffer::copyBufferToImage(const VulkanBuffer& srcBuffer, const VulkanImage& dstImage, const std::vector<VkBufferImageCopy>& pRegions)
    {
        vkCmdCopyBufferToImage(cmd, srcBuffer.get(), dstImage.get(), dstImage.getLayout(), static_cast<uint32_t>(pRegions.size()), pRegions.data());
    }

    //---------------------------------------------------------------------------
    //  Public Methods - Image Functions
    //---------------------------------------------------------------------------

    void CommandBuffer::copyImageToBuffer(const VulkanImage& srcImage, const VulkanBuffer& dstBuffer)
    {
        VkBufferImageCopy imageCopy = { 0, 0, 0,{ srcImage.getAspectMask(), 0, 0, 1 }, {}, { srcImage.getWidth(), srcImage.getHeight(), 1} };
        vkCmdCopyImageToBuffer(cmd, srcImage.get(), srcImage.getLayout(), dstBuffer.get(), 1, &imageCopy);
    }

    void CommandBuffer::copyImageToBuffer(const VulkanImage& srcImage, const VulkanBuffer& dstBuffer, const std::vector<VkBufferImageCopy>& pRegions)
    {
        vkCmdCopyImageToBuffer(cmd, srcImage.get(), srcImage.getLayout(), dstBuffer.get(), static_cast<uint32_t>(pRegions.size()), pRegions.data());
    }

    // Put a command in this CommandBuffer: Copy an image with "vkCmdCopyImage"
    void CommandBuffer::copyImage(const VulkanImage& srcImage, const VulkanImage& dstImage, uint32_t baseArrayLayer, uint32_t mipLevel)
    {
        VkImageCopy copyRegion;
        copyRegion.srcSubresource.aspectMask     = srcImage.getAspectMask();
        copyRegion.srcSubresource.mipLevel       = 0;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.layerCount     = 1;
        copyRegion.srcOffset                     = { 0, 0, 0 };
        copyRegion.dstSubresource.aspectMask     = dstImage.getAspectMask();
        copyRegion.dstSubresource.mipLevel       = mipLevel;
        copyRegion.dstSubresource.baseArrayLayer = baseArrayLayer;
        copyRegion.dstSubresource.layerCount     = 1;
        copyRegion.dstOffset                     = { 0, 0, 0 };
        copyRegion.extent                        = { srcImage.getWidth(), srcImage.getHeight(), 1};

        // Put the copy command into the command buffer
        vkCmdCopyImage(cmd, srcImage.get(), srcImage.getLayout(), dstImage.get(), dstImage.getLayout(), 1, &copyRegion);
    }

    // Change the layout of an image using a Pipeline-Barrier
    void CommandBuffer::setImageLayout(VulkanImage& image, const VkImageLayout& newLayout, uint32_t baseMipLevel)
    {
        VkImageSubresourceRange subResource;
        subResource.aspectMask      = image.getAspectMask();
        subResource.baseArrayLayer  = 0;
        subResource.layerCount      = image.numLayers();
        subResource.baseMipLevel    = baseMipLevel;
        subResource.levelCount      = 1;

        setImageLayout(image.get(), image.getLayout(), newLayout, subResource);
        image.currentLayout = newLayout;
    }

    // Change the layout of an image using a Pipeline-Barrier
    void CommandBuffer::setImageLayout(VulkanImage& image, const VkImageLayout& newLayout)
    {
        VkImageSubresourceRange subResource;
        subResource.aspectMask      = image.getAspectMask();
        subResource.baseArrayLayer  = 0;
        subResource.layerCount      = image.numLayers();
        subResource.baseMipLevel    = 0;
        subResource.levelCount      = image.numMips();

        setImageLayout(image.get(), image.getLayout(), newLayout, subResource);
        image.currentLayout = newLayout;
    }

    // Change the layout of an image using a Pipeline-Barrier
    void CommandBuffer::setImageLayout(VulkanImage& image, const VkImageLayout& newLayout, const VkImageSubresourceRange& subResource)
    {
        setImageLayout(image.get(), image.getLayout(), newLayout, subResource);
        image.currentLayout = newLayout;
    }

    void CommandBuffer::setViewport(Framebuffer* fbo)
    {
        // Update dynamic viewport state
        VkViewport viewport = { 0, 0, static_cast<float>(fbo->getWidth()), static_cast<float>(fbo->getHeight()), 0.0f, 1.0f };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
    }

    void CommandBuffer::setScissor(Framebuffer* fbo)
    {
        VkRect2D scissor = { { 0,0 },{ fbo->getWidth(), fbo->getHeight() } };
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    //---------------------------------------------------------------------------
    //  Public Methods - Pipeline-Barriers
    //---------------------------------------------------------------------------

    // Put a pipeline-barrier into this cmd
    void CommandBuffer::pipelineBarrier(const VkPipelineStageFlags& srcStages, const VkPipelineStageFlags& dstStages, 
                                        const VkAccessFlags& srcAccessMask, const VkAccessFlags& dstAccessMask)
    {
        VkMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = srcAccessMask;
        barrier.dstAccessMask = dstAccessMask;

        vkCmdPipelineBarrier(cmd, srcStages, dstStages, 0, 1, &barrier, 0, NULL, 0, NULL);
    }

    //---------------------------------------------------------------------------
    //  Public Methods - PushConstants
    //---------------------------------------------------------------------------

    void CommandBuffer::pushConstants(const VkPipelineLayout& pipeLayout, const VkShaderStageFlags& shaderStage, 
                                      const uint32_t& offset, const uint32_t& size, const void* pValues)
    {
        // Offset of 64 (First matrice is for per-object data)
        vkCmdPushConstants(cmd, pipeLayout, shaderStage, offset, size, pValues);
    }

    //---------------------------------------------------------------------------
    //  Static Methods
    //---------------------------------------------------------------------------

    // Submit a bunch of command buffers to the given queue
    void CommandBuffer::submit(VkQueue queue, const std::vector<const CommandBuffer*>& commandBuffers, const VulkanFence* fence)
    {
        submit(queue, commandBuffers, {}, {}, {}, fence);
    }

    // Submit a bunch of command buffers to the given queue
    void CommandBuffer::submit(VkQueue queue, const std::vector<const CommandBuffer*>& commandBuffers, 
                               const std::vector<const VulkanSemaphore*>& waitSems, const std::vector<VkPipelineStageFlags>& waitDstStageMasks, 
                               const std::vector<const VulkanSemaphore*>& signalSems, const VulkanFence* fence)
    {
        assert(waitSems.size() == waitDstStageMasks.size());

        auto cmds               = getVkList<VkCommandBuffer>(commandBuffers);
        auto waitSemaphores     = getVkList<VkSemaphore>(waitSems);
        auto signalSemaphores   = getVkList<VkSemaphore>(signalSems);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext                = nullptr;
        submitInfo.waitSemaphoreCount   = static_cast<uint32_t>(waitSemaphores.size());
        submitInfo.pWaitSemaphores      = waitSemaphores.data();
        submitInfo.pWaitDstStageMask    = waitDstStageMasks.data();
        submitInfo.commandBufferCount   = static_cast<uint32_t>(cmds.size());
        submitInfo.pCommandBuffers      = cmds.data();
        submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
        submitInfo.pSignalSemaphores    = signalSemaphores.data();

        VkResult res = vkQueueSubmit(queue, 1, &submitInfo, fence ? fence->get() : VK_NULL_HANDLE);
        assert(res == VK_SUCCESS);
    }


    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Put a vkCmdPipelineBarrier into this Command Buffer. Find srcAccess- and dstAccessMask automatically based on the given layouts.
    void CommandBuffer::setImageLayout(const VkImage& image, const VkImageAspectFlags& aspectMask, const VkImageLayout& oldLayout, const VkImageLayout& newLayout)
    {
        setImageLayout(image, oldLayout, newLayout, { aspectMask, 0, 1, 0, 1 });
    }

    // Put a vkCmdPipelineBarrier into this Command Buffer. Find srcAccess- and dstAccessMask automatically based on the given layouts.
    void CommandBuffer::setImageLayout(const VkImage& image, const VkImageLayout& oldLayout, const VkImageLayout& newLayout, const VkImageSubresourceRange& subResource)
    {
        VkAccessFlags srcAccessMask = 0;
        VkAccessFlags dstAccessMask = 0;

        // Undefined layout
        // Only allowed as initial layout!
        // Make sure any writes to the image have been finished
        if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
        {
            srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        }

        // Old layout is color attachment
        // Make sure any writes to the color buffer have been finished
        if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        }

        // Old layout is depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }

        // Old layout is transfer source
        // Make sure any reads from the image have been finished
        if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        }

        // Old layout is shader read (sampler, input attachment)
        // Make sure any shader reads from the image have been finished
        if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        }

        // Old layout is transfer destination optimal
        // Make sure any shader writes have been finished
        if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        }

        // Old layout is present
        // Make sure any reads to the image have been finished
        if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        }

        // Target layouts (new)

        // New layout is transfer destination (copy, blit)
        // Make sure any copyies to the image have been finished
        if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        }

        // New layout is transfer source (copy, blit)
        // Make sure any reads from and writes to the image have been finished
        if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        }

        // New layout is color attachment
        // Make sure any writes to the color buffer hav been finished
        if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            // srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        }

        // New layout is depth attachment
        // Make sure any writes to depth/stencil buffer have been finished
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            dstAccessMask = dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }

        // New layout is shader read (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        }

        // New layout is present
        // Make sure any reads to the image have been finished
        if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        }

        // Reset srcAccessMask if image layout is undefined
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
        {
            srcAccessMask = 0;
        }

        setImageLayout(image, oldLayout, newLayout, srcAccessMask, dstAccessMask, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                       VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, subResource);
    }

    // Put a vkCmdPipelineBarrier into the given Command Buffer. Queue Familys will be ignored. Src and dst Stage are TOP_OF_PIPE.
    void CommandBuffer::setImageLayout(const VkImage& image, const VkImageAspectFlags& aspectMask, const VkImageLayout& oldLayout, const VkImageLayout& newLayout,
        const VkAccessFlags& srcAccessMask, const VkAccessFlags& dstAccessMask)
    {
        setImageLayout(image, aspectMask, oldLayout, newLayout, srcAccessMask, dstAccessMask, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    }

    // Put a vkCmdPipelineBarrier into the given Command Buffer. Queue Familys will be ignored.
    void CommandBuffer::setImageLayout(const VkImage& image, const VkImageAspectFlags& aspectMask, const VkImageLayout& oldLayout, const VkImageLayout& newLayout,
        const VkAccessFlags& srcAccessMask, const VkAccessFlags& dstAccessMask,
        const VkPipelineStageFlags& srcStages, const VkPipelineStageFlags& dstStages)
    {
        setImageLayout(image, oldLayout, newLayout, srcAccessMask, dstAccessMask,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, srcStages, dstStages, { aspectMask, 0, 1, 0, 1 });
    }

    // Put a vkCmdPipelineBarrier into the given Command Buffer. Set everything manual.
    void CommandBuffer::setImageLayout(const VkImage& image, const VkImageAspectFlags& aspectMask, const VkImageLayout& oldLayout, const VkImageLayout& newLayout,
        const VkAccessFlags& srcAccessMask, const VkAccessFlags& dstAccessMask,
        const uint32_t& srcQueueFamily, const uint32_t& dstQueueFamily,
        const VkPipelineStageFlags& srcStages, const VkPipelineStageFlags& destStages)
    {
        setImageLayout(image, oldLayout, newLayout, srcAccessMask, dstAccessMask,
            srcQueueFamily, dstQueueFamily, srcStages, destStages, { aspectMask, 0, 1, 0, 1 });
    }

    // Put a vkCmdPipelineBarrier into the given Command Buffer. Set everything manual.
    void CommandBuffer::setImageLayout(const VkImage& image, const VkImageLayout& oldLayout, const VkImageLayout& newLayout,
        const VkAccessFlags& srcAccessMask, const VkAccessFlags& dstAccessMask,
        const uint32_t& srcQueueFamily, const uint32_t& dstQueueFamily,
        const VkPipelineStageFlags& srcStages, const VkPipelineStageFlags& destStages, const VkImageSubresourceRange& subResource)
    {
        VkImageMemoryBarrier imageMemoryBarrier;
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.pNext = nullptr;
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = dstAccessMask;
        imageMemoryBarrier.oldLayout = oldLayout;
        imageMemoryBarrier.newLayout = newLayout;
        imageMemoryBarrier.srcQueueFamilyIndex = srcQueueFamily;
        imageMemoryBarrier.dstQueueFamilyIndex = dstQueueFamily;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subResource;

        vkCmdPipelineBarrier(cmd, srcStages, destStages, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
    }


}