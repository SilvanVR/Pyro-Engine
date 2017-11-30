#include "framebuffer.h"

#include "vulkan-core/pipelines/renderpass/renderpass.h"
#include "vulkan-core/vulkan_base.h"


namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Framebuffer::Framebuffer(VkDevice _device, Renderpass* renderpass, uint32_t _width, uint32_t _height,
                             const std::vector<std::shared_ptr<VulkanImageView>>& imgViews, const std::shared_ptr<VulkanImageView>& _depthView)
        : device(_device), width(_width), height(_height), colorImageViews(imgViews), depthView(_depthView)
    {
        initVkFramebuffer(renderpass);
    }

    Framebuffer::Framebuffer(VkDevice _device, Renderpass* renderpass, uint32_t _width, uint32_t _height,
                             const std::vector<VkImageUsageFlags>& colorUsages, VkImageUsageFlags depthUsage)
        : device(_device), width(_width), height(_height)
    {
        VkFormat depthFormat = renderpass->getDepthFormat();
        bool includeDepth = depthFormat != VK_FORMAT_UNDEFINED ? true : false;

        auto cmd = VulkanBase::getCommandPool()->allocate();
        cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        // Color-Attachments
        std::vector<Renderpass::Attachment> colorAttachments = renderpass->getColorAttachments();
        assert(colorAttachments.size() == colorUsages.size() && "ERROR: Amount of color-Usages must be the same as the amount"
                                                                "of color-attachments in the given renderpass!");

        for (uint32_t i = 0; i < static_cast<uint32_t>(colorAttachments.size()); i++)
        {
            // Create an image for every attachment
            std::unique_ptr<VulkanImage> colorImage(new VulkanImage(device, Vec2ui(width, height), colorAttachments[i].format, colorUsages[i]));

            // Create an image-view for that image
            std::shared_ptr<VulkanImageView> imageView(new VulkanImageView(device, *colorImage, VK_IMAGE_VIEW_TYPE_2D));

            // Transition the layout if needed
            if(colorAttachments[i].initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)
                cmd->setImageLayout(*colorImage, colorAttachments[i].initialLayout);

            // Save both smart-pointers
            colorImageViews.push_back(std::move(imageView));
            colorImages.push_back(std::move(colorImage));
        }

        // Depth-Attachment
        if (includeDepth)
        {
            // Create an depth-image
            depthImage = std::unique_ptr<VulkanImage>(new VulkanImage(device, Vec2ui(width, height), depthFormat, depthUsage));

            // Create a image-view for the depth-image
            depthView = std::shared_ptr<VulkanImageView>(new VulkanImageView(device, *depthImage, VK_IMAGE_VIEW_TYPE_2D));

            // Transition layout
            if (renderpass->getDepthLayout() == VK_IMAGE_LAYOUT_UNDEFINED)
                cmd->setImageLayout(*depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            else
                cmd->setImageLayout(*depthImage, renderpass->getDepthLayout());
        }

        // Submit layout transitions
        cmd->endSubmitAndWaitForFence(device, VulkanBase::getGraphicQueue());

        // Create the VkFramebuffer
        initVkFramebuffer(renderpass);
    }


    Framebuffer::Framebuffer(VkDevice _device, Renderpass* renderpass, uint32_t _width, uint32_t _height,
                             const std::vector<VkImageUsageFlags>& colorUsages, const std::shared_ptr<VulkanImageView>& _depthView)
        : device(_device), width(_width), height(_height), depthView(_depthView)
    {
        auto cmd = VulkanBase::getCommandPool()->allocate();
        cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        // Color-Attachments
        std::vector<Renderpass::Attachment> colorAttachments = renderpass->getColorAttachments();
        assert(colorAttachments.size() == colorUsages.size() && "ERROR: Amount of color-Usages must be the same as the amount"
                                                                "of color-attachments in the given renderpass!");

        for (uint32_t i = 0; i < static_cast<uint32_t>(colorAttachments.size()); i++)
        {
            std::unique_ptr<VulkanImage> colorImage(new VulkanImage(device, Vec2ui(width, height), colorAttachments[i].format, colorUsages[i]));

            std::shared_ptr<VulkanImageView> imageView(new VulkanImageView(device, *colorImage, VK_IMAGE_VIEW_TYPE_2D));

            if (colorAttachments[i].initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)
                cmd->setImageLayout(*colorImage, colorAttachments[i].initialLayout);

            colorImageViews.push_back(std::move(imageView));
            colorImages.push_back(std::move(colorImage));
        }

        // Submit layout transitions
        cmd->endSubmitAndWaitForFence(device, VulkanBase::getGraphicQueue());

        // Create the VkFramebuffer
        initVkFramebuffer(renderpass);
    }


    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    //Destroy the VkFramebuffer object
    Framebuffer::~Framebuffer()
    {
        vkDeviceWaitIdle(device);
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    void Framebuffer::initVkFramebuffer(const Renderpass* renderpass)
    {
        std::vector<VkImageView> imageViews;

        for (const auto& view : colorImageViews)
            imageViews.push_back(view->get());

        if (depthView)
            imageViews.push_back(depthView->get());

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.pNext           = nullptr;
        framebufferInfo.flags           = 0;
        framebufferInfo.renderPass      = renderpass->get();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
        framebufferInfo.pAttachments    = imageViews.data();
        framebufferInfo.width           = this->width;
        framebufferInfo.height          = this->height;
        framebufferInfo.layers          = 1;

        VkResult res = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
        assert(res == VK_SUCCESS);
    }


}