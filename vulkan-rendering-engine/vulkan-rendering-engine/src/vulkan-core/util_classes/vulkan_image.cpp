#include "vulkan_image.h"

#include "vulkan-core/memory_management/vulkan_memory_manager.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "vulkan-core/vulkan_base.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    VulkanImage::VulkanImage(const VkImage& _image, const VkExtent3D& extent, const VkFormat& _format, bool preInitialized)
        : image(_image), size(extent), format(_format)
    {
        if (preInitialized)
            currentLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        setAspectMask();
    }

    VulkanImage::VulkanImage(VkDevice device, const Vec2ui& size, const VkFormat& format, const VkImageUsageFlags& usage, bool preInitialized)
        : VulkanImage(device, { size.x(), size.y(), 1}, format, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, preInitialized)
    {}

    VulkanImage::VulkanImage(VkDevice _device, const VkExtent3D& extent, const VkFormat& _format, const VkImageUsageFlags& _usage,
                             const VkFlags& requirementsMask, bool preInitialized, const VkImageTiling& tiling, const VkImageCreateFlags& flags,
                             const uint32_t& mipLevels, const uint32_t& arrayLayers, const VkSampleCountFlagBits& samples, const VkImageType& imageType)
        : device(_device), size(extent), format(_format), usage(_usage), numMipLevels(mipLevels), numArrayLayers(arrayLayers)
    {
        if (preInitialized)
            currentLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext                 = nullptr;
        imageInfo.flags                 = flags;
        imageInfo.imageType             = imageType;
        imageInfo.format                = format;
        imageInfo.extent                = size;
        imageInfo.mipLevels             = numMipLevels;
        imageInfo.arrayLayers           = numArrayLayers;
        imageInfo.samples               = samples;
        imageInfo.tiling                = tiling;
        imageInfo.usage                 = usage;
        imageInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.queueFamilyIndexCount = 0;
        imageInfo.pQueueFamilyIndices   = nullptr;
        imageInfo.initialLayout         = currentLayout;

        createImage(imageInfo, requirementsMask);

        setAspectMask();
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    VulkanImage::~VulkanImage()
    {
        if (device != VK_NULL_HANDLE)
        {
            vkDeviceWaitIdle(device);
            VMM::freeMemory(mem);
            vkDestroyImage(device, image, nullptr);
        }
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    uint32_t VulkanImage::getWidth(uint32_t mipLevel) const
    {
        assert(mipLevel < numMipLevels);
        return size.width >> mipLevel;
    }

    uint32_t VulkanImage::getHeight(uint32_t mipLevel) const
    {
        assert(mipLevel < numMipLevels);
        return size.height >> mipLevel;
    }

    // Push the given data into this texture-object via staging.
    void VulkanImage::push(const void* data, uint32_t size, uint32_t offset)
    {
        uint32_t sizeInBytes = getWidth() * getHeight() * vkTools::getBytesPerPixel(getFormat());

        VulkanBuffer stagingBuffer(device, sizeInBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        stagingBuffer.copyInto(data, size, offset);

        const VkImageLayout& currentLayout = getLayout();

        auto cmd = VulkanBase::getCommandPool()->allocate();
        cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        cmd->setImageLayout(*this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        cmd->copyBufferToImage(stagingBuffer, *this);
        cmd->setImageLayout(*this, currentLayout);

        cmd->endSubmitAndWaitForFence(device, VulkanBase::getGraphicQueue());
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Set the correct aspect-mask based on the image-usage and format
    void VulkanImage::setAspectMask()
    {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT)
            {
                // Aspect mask depends on use-case of the Image-View
                //aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            else if (format == VK_FORMAT_S8_UINT)
                aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    void VulkanImage::createImage(const VkImageCreateInfo& createInfo, const VkFlags& memoryFlags)
    {
        VkResult res = vkCreateImage(device, &createInfo, nullptr, &image);
        assert(res == VK_SUCCESS);

        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(device, image, &memReqs);

        // Allocate memory
        mem = VMM::allocateMemory(memReqs, memoryFlags);

        // Bind memory
        res = vkBindImageMemory(device, image, mem, 0);
        assert(res == VK_SUCCESS);
    }

    //---------------------------------------------------------------------------
    //  VulkanImageView class - Constructor
    //---------------------------------------------------------------------------

    VulkanImageView::VulkanImageView(VkDevice _device, const VulkanImage& image, const VkImageViewType& viewType)
        : VulkanImageView(_device, image, viewType, image.getFormat())
    {}

    VulkanImageView::VulkanImageView(VkDevice _device, const VulkanImage& _image, const VkImageViewType& viewType, const VkFormat& format)
        : device(_device), image(&_image)
    {
        createImageView2D(image->get(), viewType, format, image->getAspectMask(), image->numMips());
    }

    VulkanImageView::VulkanImageView(VkDevice _device, const VulkanImage& _image, const VkImageViewType& viewType, const VkFormat& format,
                                     const VkImageSubresourceRange& subresourceRange, const VkComponentMapping& components)
        : device(_device), image(&_image)
    {
        createImageView2D(image->get(), viewType, format, subresourceRange, components);
    }

    //---------------------------------------------------------------------------
    //  VulkanImageView class - Destructor
    //---------------------------------------------------------------------------

    VulkanImageView::~VulkanImageView()
    {
        vkDeviceWaitIdle(device);
        vkDestroyImageView(device, view, nullptr);
    }

    //---------------------------------------------------------------------------
    //  VulkanImageView class - Public Methods
    //---------------------------------------------------------------------------


    //---------------------------------------------------------------------------
    //  VulkanImageView class - Private Methods
    //---------------------------------------------------------------------------

    void VulkanImageView::createImageView2D(const VkImage& image, const VkImageViewType& viewType, const VkFormat& format, 
                                            const VkImageAspectFlags& aspectMask, uint32_t mipLevelCount)
    {
        VkImageSubresourceRange subresourceRange = { aspectMask, 0, mipLevelCount, 0, VK_REMAINING_ARRAY_LAYERS};
        createImageView2D(image, viewType, format, subresourceRange);
    }

    void VulkanImageView::createImageView2D(const VkImage& image, const VkImageViewType& viewType, const VkFormat& format, const VkImageSubresourceRange& subresourceRange)
    {
        const VkComponentMapping components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
        createImageView2D(image, viewType, format, subresourceRange, components);
    }

    // Creates an Vulkan Image view.
    void VulkanImageView::createImageView2D(const VkImage& image, const VkImageViewType& viewType, const VkFormat& format,
                                            const VkImageSubresourceRange& subresourceRange, const VkComponentMapping& components)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.pNext            = nullptr;
        viewInfo.flags            = 0;
        viewInfo.image            = image;
        viewInfo.viewType         = viewType;
        viewInfo.format           = format;
        viewInfo.components       = components;
        viewInfo.subresourceRange = subresourceRange;

        // Create the image view
        VkResult res = vkCreateImageView(device, &viewInfo, nullptr, &view);
        assert(res == VK_SUCCESS);
    }


}