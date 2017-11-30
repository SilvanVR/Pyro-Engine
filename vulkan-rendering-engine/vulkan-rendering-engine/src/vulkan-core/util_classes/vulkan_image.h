#ifndef VULKAN_IMAGE_H_
#define VULKAN_IMAGE_H_

// Contains the classes VulkanImage + VulkanImageView

#include "build_options.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  VulkanImage class
    //---------------------------------------------------------------------------

    // This class Encapsulates a VkImage and VkImageView - object
    class VulkanImage
    {
        friend class Renderpass; // Allow the renderpass to change the current-layout during begin() and end()
        friend class CommandBuffer; // Allow a command buffer to access the private fields
        friend class VulkanImageView; // Allow the image-view class to acess the private fields

    public:
        // Constructor which take in a VkImage directly. Only used for special cases.
        VulkanImage(const VkImage& image, const VkExtent3D& extent, const VkFormat& format, bool preInitialized = false);

        // Basic Constructors
        VulkanImage(VkDevice device, const Vec2ui& size,
                    const VkFormat& format              = VK_FORMAT_UNDEFINED,
                    const VkImageUsageFlags& usage      = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    bool preInitialized                 = false);

        VulkanImage(VkDevice _device, const VkExtent3D& extent, const VkFormat& _format, const VkImageUsageFlags& usage,
                    const VkFlags& requirementsMask      = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    bool preInitialized                  = false,
                    const VkImageTiling& tiling          = VK_IMAGE_TILING_OPTIMAL,
                    const VkImageCreateFlags& flags      = 0,
                    const uint32_t& mipLevels            = 1,
                    const uint32_t& arrayLayers          = 1,
                    const VkSampleCountFlagBits& samples = VK_SAMPLE_COUNT_1_BIT,
                    const VkImageType& imageType         = VK_IMAGE_TYPE_2D);

        ~VulkanImage();

        uint32_t             getWidth(uint32_t mipLevel = 0) const;
        uint32_t             getHeight(uint32_t mipLevel = 0) const;
        VkFormat             getFormat()     const   { return format; }
        VkImageLayout        getLayout()     const   { return currentLayout; }
        uint32_t             numMips()       const   { return numMipLevels; }
        uint32_t             numLayers()     const   { return numArrayLayers; }
        VkImageUsageFlags    getUsage()      const   { return usage; }
        VkImageAspectFlags   getAspectMask() const   { return aspectMask; }

        // Push the given data into this texture-object (on the gpu) via staging.
        void push(const void* data, uint32_t size = WHOLE_BUFFER_SIZE, uint32_t offset = 0);

    private:
        VulkanImage(const VulkanImage& other) = delete;
        VulkanImage& operator=(const VulkanImage& other) = delete;

        VkDevice                device          = VK_NULL_HANDLE;
        VkExtent3D              size;

        uint32_t                numArrayLayers  = 1;
        uint32_t                numMipLevels    = 1;
        VkFormat                format          = VK_FORMAT_UNDEFINED;
        VkImageLayout           currentLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageAspectFlags      aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        VkImage                 image           = VK_NULL_HANDLE;
        VkDeviceMemory          mem             = VK_NULL_HANDLE;
        VkImageUsageFlags       usage           = VK_NULL_HANDLE;

        // Return the VkImage from this class
        const VkImage& get() const { return image; }

        // Create the VkImage
        void createImage(const VkImageCreateInfo& createInfo, const VkFlags& memoryFlags);

        // Set the correct aspect-mask based on the image-usage and format
        void setAspectMask();
    };


    //---------------------------------------------------------------------------
    //  VulkanImageView class
    //---------------------------------------------------------------------------

    class VulkanImageView
    {

    public:
        // Normal Constructors
        VulkanImageView(VkDevice device, const VulkanImage& image, const VkImageViewType& viewType = VK_IMAGE_VIEW_TYPE_2D);
        VulkanImageView(VkDevice device, const VulkanImage& image, const VkImageViewType& viewType, const VkFormat& format);
        VulkanImageView(VkDevice device, const VulkanImage& image, const VkImageViewType& viewType, const VkFormat& format, 
                        const VkImageSubresourceRange& subresourceRange, const VkComponentMapping& components);

        ~VulkanImageView();

        // Return the VkImageView this class encapsulates
        const VkImageView& get() const { return view; }

        // Return the VulkanImage-Object from which this is a view
        const VulkanImage* getImage() const { return image; }

    private:
        VkDevice                device      = VK_NULL_HANDLE;
        VkImageView             view        = VK_NULL_HANDLE;
        const VulkanImage*      image       = nullptr;

        // Creates the VkImageView
        void createImageView2D(const VkImage& image, const VkImageViewType& viewType, const VkFormat& format, 
                               const VkImageAspectFlags& aspectMask, uint32_t mipLevelCount);
        void createImageView2D(const VkImage& image, const VkImageViewType& viewType, const VkFormat& format, const VkImageSubresourceRange& subresourceRange);
        void createImageView2D(const VkImage& image, const VkImageViewType& viewType, const VkFormat& format, 
                               const VkImageSubresourceRange& subresourceRange, const VkComponentMapping& components);
    };


}



#endif // !VULKAN_IMAGE_H_

