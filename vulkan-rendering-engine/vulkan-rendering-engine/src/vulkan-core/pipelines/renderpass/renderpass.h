/*
*  Renderpass-Class header file.
*  Is an abstraction of an Vulkan Renderpass with different subpasses and/or attachments.
*
*  Date:    23.04.2016
*  Creator: Silvan Hau
*/

#ifndef RENDERPASS_H_
#define RENDERPASS_H_

#include "build_options.h"

namespace Pyro
{
    class Framebuffer;

    //---------------------------------------------------------------------------
    //  Renderpass class
    //---------------------------------------------------------------------------

    class Renderpass
    {
        friend class Framebuffer;       // Allow the Framebuffer to access the VkRenderpass
        friend class GraphicsPipeline;  // Allow the GraphicsPipeline to access the VkRenderpass

    public:
        struct AttachmentDescription
        {
            VkAttachmentDescription description;
            VkImageLayout           layoutDuringSubpass;
        };

        struct Attachment
        {
            VkImageLayout initialLayout;
            VkImageLayout finalLayout;
            VkFormat      format;
        };

        // Create a VkRenderPass with one color-attachment and if necessary with a depth-attachment.
        // E.G. Renderpass(device, VK_FORMAT_R8G8B8A8) or Renderpass(device, VK_FORMAT_R8G8B8A8, VK_FORMAT_D_32F)
        Renderpass(VkDevice device, const VkFormat& colorFormat, const VkFormat& depthFormat = VK_FORMAT_UNDEFINED);

        // Create a VkRenderPass with one color-Attachment
        Renderpass(VkDevice device, const AttachmentDescription& colorAttachment);

        // Create a VkRenderPass with one color-Attachment and if necessary with a depth-attachment
        Renderpass(VkDevice device, const AttachmentDescription& colorAttachment, const AttachmentDescription& depthAttachment);

        // Create a VkRenderPass with the given amount of color-Attachments and if necessary with a given depth-attachment
        Renderpass(VkDevice device, const std::vector<AttachmentDescription>& colorAttachments, const AttachmentDescription& depthAttachment);

        // Destroy the VkRenderPass
        ~Renderpass() { vkDestroyRenderPass(device, renderpass, nullptr); }


        // Record the begin of this renderpass in the given cmd
        void begin(VkCommandBuffer cmd, Framebuffer* framebuffer, const VkSubpassContents& subpassContents = VK_SUBPASS_CONTENTS_INLINE);

        // Record the end of this renderpass in the given cmd
        void end(VkCommandBuffer cmd);

        // Change clear-values for this renderpass
        void setColorClearValue(const uint32_t& attachmentIndex, const Vec4f& clearValue);
        void setDepthStencilClearValue(const Vec2f& clearValue);

        // A Framebuffer-class ask for this information to create images (and a depth-buffer)
        const std::vector<Attachment>& getColorAttachments() const { return colorAttachments; }
        const VkImageLayout& getDepthLayout() const { return depth.initialLayout; }
        const VkFormat& getDepthFormat() const { return depth.format; }

    private:
        // forbid copy and copy assignment
        Renderpass(const Renderpass& renderpass) {};
        Renderpass& operator=(const Renderpass& renderpass) {};

        VkRenderPass                renderpass;         // The VkRenderpass handle
        VkDevice                    device;             // Store a reference for destroying the VkRenderPass

        Framebuffer*                activeFramebuffer;  // Set to the given framebuffer when begin() is called

        std::vector<Attachment>     colorAttachments;   // Color format & layouts

        // Depth format & layout
        Attachment                  depth = { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED, VK_FORMAT_UNDEFINED };

        // Standard color + depth/stencil clear-value
        Vec4f                       colorClearValue          = Vec4f(0.0f, 0.0f, 0.0f, 0.0f);
        Vec2f                       depthStencilClearValue   = Vec2f(1.0f, 0.0f);

        // Cached clear-values for each attachment which will be cleared
        std::vector<VkClearValue>   clearValues;
        
        // Creates the VkRenderPass
        void createVkRenderpass(const std::vector<AttachmentDescription>& colorAttachments, const AttachmentDescription& depthAttachment);

        // Set all layouts from the corresponding attachments in the active-fbo to the layout specified in "colorAttachments"
        // "begin" = true: "initialLayout" is chosen, otherwise "finalLayout"
        void setFramebufferAttachmentLayouts(bool begin);

        // Returns the VkRenderPass
        const VkRenderPass& get() const { return renderpass; }
    };


}





#endif // !RENDERPASS_H_
