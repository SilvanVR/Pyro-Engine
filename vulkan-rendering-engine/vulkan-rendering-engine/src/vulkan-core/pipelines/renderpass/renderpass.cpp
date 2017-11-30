#include "renderpass.h"

#include "vulkan-core/pipelines/framebuffers/framebuffer.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------

    Renderpass::Renderpass(VkDevice _device, const VkFormat& colorFormat, const VkFormat& depthFormat)
        : device(_device)
    {
        bool includeDepth = depthFormat != VK_FORMAT_UNDEFINED;

        VkAttachmentDescription colorAttachment, depthAttachment = {};
        colorAttachment.flags            = 0;
        colorAttachment.format           = colorFormat;
        colorAttachment.samples          = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (includeDepth)
        {
            depthAttachment.flags            = 0;
            depthAttachment.format           = depthFormat;
            depthAttachment.samples          = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachment.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.finalLayout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        createVkRenderpass({ { colorAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } },
                           { depthAttachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
    }

    // Create a VkRenderPass with one color-Attachment
    Renderpass::Renderpass(VkDevice device, const AttachmentDescription& colorAttachment)
        : Renderpass(device, colorAttachment, {})
    {}

    Renderpass::Renderpass(VkDevice device, const AttachmentDescription& colorAttachment, const AttachmentDescription& depthAttachment)
        : Renderpass(device, std::vector<AttachmentDescription>{ colorAttachment }, depthAttachment)
    {}

    Renderpass::Renderpass(VkDevice _device, const std::vector<AttachmentDescription>& colorAttachments, const AttachmentDescription& depthAttachment)
        : device(_device)
    {
        createVkRenderpass(colorAttachments, depthAttachment);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void Renderpass::begin(VkCommandBuffer cmd, Framebuffer* framebuffer, const VkSubpassContents& subpassContents)
    {
        activeFramebuffer = framebuffer;

        // Change all layouts from the attachments in the FBO
        setFramebufferAttachmentLayouts(true);

        VkRect2D renderArea = { 0, 0, framebuffer->getWidth(), framebuffer->getHeight() };

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext           = nullptr;
        renderPassBeginInfo.renderPass      = renderpass;
        renderPassBeginInfo.renderArea      = renderArea;
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues    = clearValues.data();
        renderPassBeginInfo.framebuffer     = framebuffer->get();

        vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, subpassContents);
    }

    // Record the end of this renderpass in the given cmd
    void Renderpass::end(VkCommandBuffer cmd)
    {
        // Change all layouts from the attachments in the FBO
        setFramebufferAttachmentLayouts(false);

        vkCmdEndRenderPass(cmd);
    }

    void Renderpass::setColorClearValue(const uint32_t& attachmentIndex, const Vec4f& clearValue)
    {
        assert(attachmentIndex < clearValues.size());
        clearValues[attachmentIndex] = { clearValue.x(), clearValue.y(), clearValue.z(), clearValue.w() };
    }

    void Renderpass::setDepthStencilClearValue(const Vec2f& clearValue)
    {
        assert(depth.format != VK_FORMAT_UNDEFINED && "This Renderpass has no depth-attachment!");
        VkClearValue newDepthStencilClearVal;
        newDepthStencilClearVal.depthStencil = { clearValue.x(), static_cast<uint32_t>(clearValue.y()) };
        clearValues[clearValues.size() - 1] = newDepthStencilClearVal;
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    void Renderpass::createVkRenderpass(const std::vector<AttachmentDescription>& colorAttachments, const AttachmentDescription& depthAttachment)
    {
        bool includeDepth = depthAttachment.description.format != VK_FORMAT_UNDEFINED &&
                            depthAttachment.layoutDuringSubpass != VK_IMAGE_LAYOUT_UNDEFINED;

        std::vector<VkAttachmentDescription> attachmentDescriptions;
        std::vector<VkAttachmentReference>   colorReferences;

        // Make a <vector> of VkAttachmentDescription and <VkAttachmentReference>
        for (uint32_t i = 0; i < colorAttachments.size(); i++)
        {
            attachmentDescriptions.push_back(std::move(colorAttachments[i].description));
            colorReferences.push_back({ i, colorAttachments[i].layoutDuringSubpass });
             
            // Push a clear value on the vector. Will be ignored if attachment will be loaded.
            if (colorAttachments[i].description.loadOp != VK_ATTACHMENT_LOAD_OP_LOAD &&
                colorAttachments[i].description.loadOp != VK_ATTACHMENT_LOAD_OP_DONT_CARE)
            {
                clearValues.resize(i + 1);
                clearValues[i] = { colorClearValue.x(), colorClearValue.y(), colorClearValue.z(), colorClearValue.w() };
            }

            // Save some information if we want to create a framebuffer from this renderpass later
            this->colorAttachments.push_back({colorAttachments[i].description.initialLayout,
                                              colorAttachments[i].description.finalLayout,
                                              colorAttachments[i].description.format });
        }

        // If depth is included add it to the <vector> of VkAttachmentDescription and create a VkAttachmentReference
        VkAttachmentReference depthReference;
        if (includeDepth)
        {
            // ORDER is IMPORTANT here, so attachmentDescriptions.size() returns the correct value
            depthReference = { static_cast<uint32_t>(attachmentDescriptions.size()), depthAttachment.layoutDuringSubpass };
            attachmentDescriptions.push_back(std::move(depthAttachment.description));

            // Add the depth-stencil clear value. Will be ignored if attachmant will be loaded.
            if (depthAttachment.description.loadOp != VK_ATTACHMENT_LOAD_OP_LOAD &&
                depthAttachment.description.loadOp != VK_ATTACHMENT_LOAD_OP_DONT_CARE)
            {
                VkClearValue newDepthStencilClearVal;
                newDepthStencilClearVal.depthStencil = { depthStencilClearValue.x(), static_cast<uint32_t>(depthStencilClearValue.y()) };
                clearValues.push_back(newDepthStencilClearVal);
            }

            // Save some information if we want to create a framebuffer from this renderpass later
            depth.initialLayout = depthAttachment.description.initialLayout;
            depth.finalLayout   = depthAttachment.description.finalLayout;
            depth.format        = depthAttachment.description.format;
        }

        VkSubpassDescription subpass = {};
        subpass.flags                   = 0;
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount    = 0;
        subpass.pInputAttachments       = nullptr;
        subpass.colorAttachmentCount    = static_cast<uint32_t>(colorReferences.size());
        subpass.pColorAttachments       = colorReferences.data();
        subpass.pResolveAttachments     = nullptr;
        subpass.pDepthStencilAttachment = includeDepth ? &depthReference : nullptr;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments    = nullptr;

        //VkSubpassDependency subpassDependencys[2];
        //subpassDependencys[0].srcSubpass        = VK_SUBPASS_EXTERNAL;
        //subpassDependencys[0].dstSubpass        = 0;
        //subpassDependencys[0].srcStageMask      = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        //subpassDependencys[0].dstStageMask      = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        //subpassDependencys[0].srcAccessMask     = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_SHADER_READ_BIT;;
        //subpassDependencys[0].dstAccessMask     = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        //subpassDependencys[0].dependencyFlags   = VK_DEPENDENCY_BY_REGION_BIT;
        //
        //subpassDependencys[1].srcSubpass        = 0;
        //subpassDependencys[1].dstSubpass        = VK_SUBPASS_EXTERNAL;
        //subpassDependencys[1].srcStageMask      = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        //subpassDependencys[1].dstStageMask      = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        //subpassDependencys[1].srcAccessMask     = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        //subpassDependencys[1].dstAccessMask     = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_SHADER_READ_BIT;;
        //subpassDependencys[1].dependencyFlags   = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo createInfo = {};
        createInfo.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = 0;
        createInfo.attachmentCount  = static_cast<uint32_t>(attachmentDescriptions.size());
        createInfo.pAttachments     = attachmentDescriptions.data();
        createInfo.subpassCount     = 1;
        createInfo.pSubpasses       = &subpass;
        createInfo.dependencyCount  = 0;
        createInfo.pDependencies    = nullptr;

        VkResult res = vkCreateRenderPass(device, &createInfo, NULL, &renderpass);
        assert(res == VK_SUCCESS);
    }


    // Set all layouts from the corresponding attachments in the fbo to the layout specified in "colorAttachments"
    // "begin" = true: "initialLayout" is chosen, otherwise "finalLayout"
    void Renderpass::setFramebufferAttachmentLayouts(bool begin)
    {
        // Transition all images in framebuffer to the correct layout
        const std::vector<std::shared_ptr<VulkanImageView>>& imageViews = activeFramebuffer->getColorViews();
        for (uint32_t i = 0; i < imageViews.size(); i++)
        {
            VulkanImage* img = const_cast<VulkanImage*>(imageViews[i]->getImage());
            img->currentLayout = begin ? colorAttachments[i].initialLayout : colorAttachments[i].finalLayout;
        }

        const std::shared_ptr<VulkanImageView>& depthView = activeFramebuffer->getDepthView();
        if (depthView)
        {
            VulkanImage* depthImg = const_cast<VulkanImage*>(depthView->getImage());
            depthImg->currentLayout = begin ? depth.initialLayout : depth.finalLayout;
        }
    }

}

