#include "swapchain.h"
#include <assert.h>

#include "vulkan-core/cmd_pool_and_buffers/Command_buffer.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "vulkan-core/window/window.h"
#include "vulkan-core/vulkan_base.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Swapchain::Swapchain(const DeviceManager& deviceManager, Window* _window)
        : gpu(deviceManager.getMainGPU().gpu),
          device(deviceManager.getDevice()),
          queueFamilyGraphicsIndex(deviceManager.getQueueFamilyGraphicsIndex()),
          queueFamilyPresentingIndex(deviceManager.getQueueFamilyPresentingIndex()), 
          window(_window)
    {
        this->create();
    }

    Swapchain::~Swapchain()
    {
        /* destroy image-views */
        this->destroy();

        /* free swapchain memory */
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    //---------------------------------------------------------------------------
    //  Public Member Functions
    //---------------------------------------------------------------------------

    // Recreates the swapchain (e.g. if window size has changed) and destroys the old one
    void Swapchain::recreate()
    {
        /* destroy image-views */
        this->destroy();

        /* recreate swapchain and image views */
        this->create();
    }

    // Aquire the next presentable image Index. Wait "timeout" nanoseconds."waitSemaphore" and/or "waitFence" become signlaed when an image is available.
    VkResult Swapchain::aquireNextImageIndex(const uint64_t& timeout, const VulkanSemaphore* signalSemaphore, const VulkanFence* signalFence, uint32_t* imageIndex)
    {
        VkSemaphore sem = signalSemaphore ? signalSemaphore->get() : VK_NULL_HANDLE;
        VkFence fence = signalFence ? signalFence->get() : VK_NULL_HANDLE;
        return vkAcquireNextImageKHR(device, swapchain, timeout, sem, fence, imageIndex);
    }

    // Returns the swapchain image given by the index
    VulkanImage& Swapchain::getImage(const uint32_t& index)
    {
        assert(index <= images.size());
        return *images[index];
    }

    // Present the next Rendered Image to the surface. Waits on "waitSemaphore".
    VkResult Swapchain::queuePresent(VkQueue queue, const std::vector<VkSemaphore>& waitSemaphores, const uint32_t& imageIndex)
    {
        const uint32_t imageIndices[] = { imageIndex };

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext               = nullptr;
        presentInfo.waitSemaphoreCount  = static_cast<uint32_t>(waitSemaphores.size());
        presentInfo.pWaitSemaphores     = waitSemaphores.data();
        presentInfo.swapchainCount      = 1;
        presentInfo.pSwapchains         = &swapchain;
        presentInfo.pImageIndices       = imageIndices;
        presentInfo.pResults            = nullptr;

        return vkQueuePresentKHR(queue, &presentInfo);
    }


    //---------------------------------------------------------------------------
    //  Private Members
    //---------------------------------------------------------------------------

    // Creates the swapchain.
    void Swapchain::create()
    {
        // Create VkSwapchain and destroys old one
        createSwapchain();

        // Create ImageViews
        createImageViews();
    }

    // Creates the VkSwapchain and destroys the old one
    void Swapchain::createSwapchain()
    {
        if (device != VK_NULL_HANDLE)
            vkDeviceWaitIdle(device);

        VkSwapchainKHR oldSwapchain = swapchain;

        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, window->getVulkanSurface(), &surfaceCapabilities);

        setSurfaceFormat();
        setPresentMode();
        setSwapchainExtent2D(surfaceCapabilities, Window::getWidth(), Window::getHeight());
        setDesiredNumberOfSwapchainImages(surfaceCapabilities, swapchainPresentMode);
        setSurfaceTransform(surfaceCapabilities);

        VkSwapchainCreateInfoKHR swapchainInfo = {};
        swapchainInfo.sType             = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.pNext             = nullptr;
        swapchainInfo.flags             = 0;
        swapchainInfo.surface           = window->getVulkanSurface();
        swapchainInfo.minImageCount     = desiredNumSwapchainImages;
        swapchainInfo.imageFormat       = surfaceFormat.format;
        swapchainInfo.imageColorSpace   = surfaceFormat.colorSpace;
        swapchainInfo.imageExtent       = swapchainExtent;
        swapchainInfo.imageArrayLayers  = 1;
        swapchainInfo.imageUsage        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        std::vector<uint32_t> queueFamilies(1, queueFamilyGraphicsIndex);
        if (queueFamilyGraphicsIndex != queueFamilyPresentingIndex)
        {
            queueFamilies.push_back(queueFamilyPresentingIndex);

            swapchainInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            swapchainInfo.queueFamilyIndexCount = (uint32_t)queueFamilies.size();
            swapchainInfo.pQueueFamilyIndices   = queueFamilies.data();
        }
        else {
            swapchainInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
            swapchainInfo.queueFamilyIndexCount = 0;
            swapchainInfo.pQueueFamilyIndices   = nullptr;
        }

        swapchainInfo.preTransform      = surfaceTransform;
        swapchainInfo.compositeAlpha    = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.presentMode       = swapchainPresentMode;
        swapchainInfo.clipped           = VK_TRUE;
        swapchainInfo.oldSwapchain      = oldSwapchain;

        VkResult res = vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain);
        assert(res == VK_SUCCESS);

        if (oldSwapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
    }

    // Create for every VkImage in the swapchain an VkImageView
    void Swapchain::createImageViews()
    {
        // Get VkImages from the swapchain
        uint32_t imageCount;
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
        std::vector<VkImage> swapchainImages(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

        images.clear();
        imageViews.clear();

        // Transition image layout from VkImages from the swapchain and depthbuffer
        auto cmd = VulkanBase::getCommandPool()->allocate();

        cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        // Create an image view and set the appropriate layout for every image
        for (uint32_t i = 0; i < imageCount; i++)
        {
            // Create an container for the VkImages
            VkExtent3D extent = { swapchainExtent.width, swapchainExtent.height, 1 };
            images.push_back(std::unique_ptr<VulkanImage>(new VulkanImage(swapchainImages[i], extent, surfaceFormat.format)));

            // Set the layout to LAYOUT_PRESENT
            cmd->setImageLayout(*images[i], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

            // Create the Image View
            imageViews.push_back(std::unique_ptr<VulkanImageView>(new VulkanImageView(device, *images[i])));
        }

        // End the Command Buffer, submit it and wait on a fence for completion
        cmd->endSubmitAndWaitForFence(VulkanBase::getDevice(), VulkanBase::getGraphicQueue());
    }

    // Searches and returns the best possible format
    void Swapchain::setSurfaceFormat()
    {
        /* Query supported swapchain format-colorspace pairs for a surface */
        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, window->getVulkanSurface(), &surfaceFormatCount, NULL);
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, window->getVulkanSurface(), &surfaceFormatCount, surfaceFormats.data());

        // If the format list includes just one entry of VK_FORMAT_UNDEFINED, the surface has no preferred format. 
        if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
            surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
        else
            surfaceFormat.format = surfaceFormats[0].format;

        surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
    }

    // Returns an appropriate presentation mode (different if VSync is enabled/disabled)
    void Swapchain::setPresentMode()
    {
        /* Query supported presentation modes */
        uint32_t presentModeCount;
        std::vector<VkPresentModeKHR> presentModes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, window->getVulkanSurface(), &presentModeCount, NULL);
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, window->getVulkanSurface(), &presentModeCount, presentModes.data());

        /* FIFO is always supported */
        swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

        if (!VulkanBase::getSettings().vsync)
        {
            /* Check if Mailbox is available (best presenting mode) and/or immediate if vsync is disabled */
            for (size_t i = 0; i < presentModes.size(); i++) {
                if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                    break;
                }
                else if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
                {
                    swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                    break;
                }
            }
        }

    }

    // Returns the appropriate size for the swapchain
    void Swapchain::setSwapchainExtent2D(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, const uint32_t& width, const uint32_t& height)
    {
        // width and height are either both -1, or both not -1.
        if (surfaceCapabilities.currentExtent.width == (uint32_t)-1) {
            // If the surface size is undefined, the size is set to the size of the images requested.
            swapchainExtent.width   = width;
            swapchainExtent.height  = height;
        }
        else {
            // If the surface size is defined, the swapchain size must match
            swapchainExtent = surfaceCapabilities.currentExtent;
            window->width   = swapchainExtent.width;
            window->height  = swapchainExtent.height;
        }
    }

    // Determine the number of VkImage's to use in the swap chain 
    void Swapchain::setDesiredNumberOfSwapchainImages(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, const VkPresentModeKHR& swapchainPresentMode)
    {
        if (swapchainPresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            desiredNumSwapchainImages = 3;                                     //Try to use Tripple buffering if Mailbox mode is available
        else
            desiredNumSwapchainImages = 2;                                     //Use at least double buffering

        if ((surfaceCapabilities.maxImageCount > 0) && (desiredNumSwapchainImages > surfaceCapabilities.maxImageCount))
        {
            desiredNumSwapchainImages = surfaceCapabilities.maxImageCount;     // Application must settle for fewer images than desired
        }
    }

    // Returns the surface transform if the surface is e.g. rotated etc.
    void Swapchain::setSurfaceTransform(const VkSurfaceCapabilitiesKHR & surfaceCapabilities)
    {
        // Sometimes images must be transformed before they are presented (i.e. due to device's orienation being other than default orientation)
        // If the specified transform is other than current transform, presentation engine will transform image
        // during presentation operation; this operation may hit performance on some platforms
        // Here we don't want any transformations to occur so if the identity transform is supported use it
        // otherwise just use the same transform as current transform
        if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            surfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        else
            surfaceTransform = surfaceCapabilities.currentTransform;

    }

    // Clean up vulkan resources in a specific manner
    void Swapchain::destroy()
    {
        /* destroy image views */
        imageViews.clear();
    }

}