/*
*  Swapchain-Class header file.
*  Represents a collection of Images (with a depthbuffer) in which the renderer will draw.
*
*  Date:    23.04.2016
*  Creator: Silvan Hau
*/

#ifndef SWAPCHAIN_H_
#define SWAPCHAIN_H_

#include "build_options.h"

#include "vulkan-core/util_classes/vulkan_image.h"
#include "vulkan-core/util_classes/vulkan_other.h"

namespace Pyro
{

    class Window;
    class CommandBuffer;
    class DeviceManager;

    //---------------------------------------------------------------------------
    //  Swapchain class
    //---------------------------------------------------------------------------
    class Swapchain
    {
    public:
        Swapchain(const DeviceManager& deviceManager, Window* window);
        ~Swapchain();

        // Returns the vulkan swapchain
        const VkSwapchainKHR&       get() const { return swapchain; }

        // Returns the currently used surface format
        const VkSurfaceFormatKHR&   getSurfaceFormat() const { return surfaceFormat; }

        // Returns one of the VkImages given by the index
        VulkanImage&                getImage(const uint32_t& index);

        // Recreates the swapchain (e.g. if window size has changed)
        void recreate();

        // Aquire the next presentable image Index. Wait "timeout" nanoseconds on "waitSemaphore" and/or "waitFence".
        VkResult aquireNextImageIndex(const uint64_t& timeout, const VulkanSemaphore* waitSemaphore, const VulkanFence* waitFence, uint32_t* imageIndex);

        // Present the next Rendered Image to the surface. Waits on "waitSemaphore".
        VkResult queuePresent(VkQueue queue, const std::vector<VkSemaphore>& waitSemaphores, const uint32_t& imageIndex);

    private:
        VkSwapchainKHR                                  swapchain = VK_NULL_HANDLE; //The Vulkan Swapchain handle
        std::vector<std::unique_ptr<VulkanImage>>       images;                     //The VulkanImage class encapsulates the VkImage from the presentation-engine
        std::vector<std::unique_ptr<VulkanImageView>>   imageViews;                 //VkImageViews from the swapchain VkImages

        // Basic Surface Queries which will be filled on every swapchain creation
        VkSurfaceFormatKHR              surfaceFormat;
        VkPresentModeKHR                swapchainPresentMode;
        VkExtent2D                      swapchainExtent;
        uint32_t                        desiredNumSwapchainImages;
        VkSurfaceTransformFlagBitsKHR   surfaceTransform;
        uint32_t                        queueFamilyGraphicsIndex;
        uint32_t                        queueFamilyPresentingIndex;

        VkPhysicalDevice                gpu = VK_NULL_HANDLE;       // Save a reference for recreating the swapchain
        VkDevice                        device = VK_NULL_HANDLE;    // Save a reference for recreating the swapchain
        Window*                         window = nullptr;           // The Window handle

        // Creates the swapchain, image-views and device local depth-buffer
        void create();

        // Creates the VkSwapchain and destroys the old one
        void createSwapchain();

        // Create for every VkImage in the swapchain an VkImageView
        void createImageViews();

        // Searches and set the best possible Surface Format
        void setSurfaceFormat();

        // Set an appropriate presentation mode (different if VSync is enabled/disabled)
        void setPresentMode();

        // Set the appropriate size for the swapchain
        void setSwapchainExtent2D(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, const uint32_t& width, const uint32_t& height);

        // Determine the number of VkImage's to use in the swap chain 
        void setDesiredNumberOfSwapchainImages(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, const VkPresentModeKHR& swapchainPresentMode);

        // Determine the surface transform if the surface is e.g. rotated etc.
        void setSurfaceTransform(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

        // Clean up vulkan resources
        void destroy();

    };

}

#endif // !SWAPCHAIN_H_
