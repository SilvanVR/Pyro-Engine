/*
*  Framebuffers-Class header file.
*  Represents an abstraction of a VkFramebuffer.
*
*  Date:    27.04.2016
*  Author:  Silvan Hau
*/

#ifndef FRAMEBUFFERS_H_
#define FRAMEBUFFERS_H_

#include "build_options.h"
#include "vulkan-core/util_classes/vulkan_image.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Framebuffer class
    //---------------------------------------------------------------------------

    class Framebuffer
    {
        friend class Renderpass; // Allow the renderpass to access the private "VkFramebuffer"

    public:
        // Constructor
        Framebuffer(VkDevice _device, Renderpass* renderpass, uint32_t width, uint32_t height, 
                    const std::vector<std::shared_ptr<VulkanImageView>>& imageViews, const std::shared_ptr<VulkanImageView>& depthView = nullptr);

        // Create a VkFramebuffer AND X-Color-Images (and optional a depth-attachment) with the specified size.
        // Information like the format or the first-layout is fetched from the Renderpass-Class.
        Framebuffer(VkDevice _device, Renderpass* renderpass, uint32_t width, uint32_t height, 
                    const std::vector<VkImageUsageFlags>& colorUsages, VkImageUsageFlags depthUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        // Create a VkFramebuffer AND X-Color-Images with the specified size.
        // Instead of creating an own depth-buffer it take the image-view from it as an extra parameter.
        // Information like the format or the first-layout is fetched from the Renderpass-Class.
        Framebuffer(VkDevice _device, Renderpass* renderpass, uint32_t width, uint32_t height,
                    const std::vector<VkImageUsageFlags>& colorUsages, const std::shared_ptr<VulkanImageView>& depthView);

        // Destructor
        ~Framebuffer();


        // Get width or height from this framebuffer. 
        const uint32_t&         getWidth() const { return width; }
        const uint32_t&         getHeight() const { return height; }

        // Return all image-views for the color-images in this framebuffer
        const std::vector<std::shared_ptr<VulkanImageView>>& getColorViews(){ return colorImageViews; }

        // Get the color / depth attachment from this frame-buffer.
        VulkanImage&            getColorImage(uint32_t index = 0){ return *colorImages[index]; }
        VulkanImage&            getDepthImage(){ return *depthImage; }

        // Get the corresponding image-views for a color or the depth-attachment
        const std::shared_ptr<VulkanImageView>& getColorView(const uint32_t& index = 0) const { return colorImageViews[index]; }
        const std::shared_ptr<VulkanImageView>& getDepthView() const { return depthView; }

    private:
        // forbid copy and copy assignment
        Framebuffer(const Framebuffer& framebuffer);
        Framebuffer& operator=(const Framebuffer& framebuffer) {};

        VkFramebuffer           framebuffer;                // The vkFramebuffer
        uint32_t                width, height;              // Width + Height from this framebuffer
        VkDevice                device = VK_NULL_HANDLE;    // Need a reference for destroying

        // If special constructor was used, it creates automatically color (+depth) attachment(s) in this class and stores it here
        std::vector<std::unique_ptr<VulkanImage>>   colorImages;
        std::unique_ptr<VulkanImage>                depthImage;

        // Shared-Ptr because some framebuffers might share the same color and/or depth-images
        std::vector<std::shared_ptr<VulkanImageView>>   colorImageViews;
        std::shared_ptr<VulkanImageView>                depthView;

        // Creates the VkFramebuffer
        void initVkFramebuffer(const Renderpass* renderpass);

        // Get the VkFramebuffers
        const VkFramebuffer& get() const { return framebuffer; }
    };

}


#endif // !FRAMEBUFFERS_H_
