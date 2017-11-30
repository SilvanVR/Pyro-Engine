#include "vulkan_texture_resource.h"

#include "material/texture/texture.h"
#include "vulkan-core/vulkan_base.h"
#include "vulkan-core/vkTools/vk_tools.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    VulkanTextureResource::VulkanTextureResource(Texture* texture, void* data, uint32_t size)
    {
        bool pushTexDataToGPU = data != nullptr;
        initTexture(texture, pushTexDataToGPU);
        if (pushTexDataToGPU)
            loadTexDataIntoGPU(texture, data, size);
        initSampler(texture);
    }

    VulkanTextureResource::VulkanTextureResource(const VkDescriptorImageInfo& imageInfo)
    {
        image = VK_NULL_HANDLE;
        this->imageInfo = imageInfo;
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    VulkanTextureResource::~VulkanTextureResource()
    {
    }

    //---------------------------------------------------------------------------
    //  Private Member
    //---------------------------------------------------------------------------

    // Create the texture on the gpu
    void VulkanTextureResource::initTexture(Texture* tex, bool pushTexDataToGPU)
    {
        // 1.) Allocate the memory on the GPU
        uint32_t mipLevels = static_cast<uint32_t>(tex->m_mipmaps.size());
        uint32_t numLayers = static_cast<uint32_t>(tex->m_layerCount);

        // Create the VkImage
        const VkExtent3D    size                = { tex->m_mipmaps[0].width, tex->m_mipmaps[0].height, 1 };
        VkImageUsageFlags   usage               = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        if(numLayers == 1 && !pushTexDataToGPU) usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VkImageCreateFlags  flags               = numLayers == 1 ? 0 : VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        VkImageTiling       tiling              = VK_IMAGE_TILING_OPTIMAL;
        VkFlags             requirementsMask    = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        bool                preInitialized      = pushTexDataToGPU;

        VulkanImage* newImage = new VulkanImage(VulkanBase::getDevice(), size, tex->getFormat(), usage, requirementsMask,
                                                preInitialized, tiling, flags, mipLevels, numLayers);
        image = std::unique_ptr<VulkanImage>(newImage);

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel   = 0;
        subresourceRange.levelCount     = mipLevels;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount     = numLayers;

        // 2.) Transition Layout to Shader_Read_Optimal
        auto cmd = VulkanBase::getCommandPool()->allocate();

        cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        cmd->setImageLayout(*image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);
        cmd->endSubmitAndWaitForFence(VulkanBase::getDevice(), VulkanBase::getGraphicQueue());

        // Create a ImageView for this texture
        VkImageViewType viewType = numLayers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_CUBE;
        VulkanImageView* newImageView = new VulkanImageView(VulkanBase::getDevice(), *image, viewType);
        view = std::unique_ptr<VulkanImageView>(newImageView);

        // Setup descriptor image info
        imageInfo.imageLayout = image->getLayout();
        imageInfo.imageView = view->get();
    }

    // Load the texture data into memory
    void VulkanTextureResource::loadTexDataIntoGPU(Texture* tex, void* data, uint32_t size)
    {
        assert(size != 0);

        auto cmd = VulkanBase::getCommandPool()->allocate();

        cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        // Create a host-visible staging buffer that contains the raw image data
        VulkanBuffer stagingBuffer(VulkanBase::getDevice(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        // Copy texture data into staging buffer
        stagingBuffer.copyInto(data);

        // Setup buffer copy regions for each mip level
        std::vector<VkBufferImageCopy> bufferCopyRegions;
        uint32_t offset = 0;
        uint32_t mipLevels = tex->numMips();

        for (uint32_t i = 0; i < mipLevels; i++)
        {
            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel       = i;
            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
            bufferCopyRegion.imageSubresource.layerCount     = tex->m_layerCount;
            bufferCopyRegion.imageExtent.width               = tex->m_mipmaps[i].width;
            bufferCopyRegion.imageExtent.height              = tex->m_mipmaps[i].height;
            bufferCopyRegion.imageExtent.depth               = 1;
            bufferCopyRegion.bufferOffset                    = offset;

            bufferCopyRegions.push_back(bufferCopyRegion);

            offset += static_cast<uint32_t>(tex->m_mipmaps[i].size) * tex->m_layerCount;
        }

        // Image barrier for optimal image (target). Optimal image will be used as destination for the copy
        cmd->setImageLayout(*image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Copy mip levels from staging buffer
        cmd->copyBufferToImage(stagingBuffer, *image, bufferCopyRegions);

        // Change texture image layout to shader read after all mip levels have been copied
        cmd->setImageLayout(*image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // Submit command buffer containing copy and image layout commands
        cmd->endSubmitAndWaitForFence(VulkanBase::getDevice(), VulkanBase::getGraphicQueue());
    }

    // Initialize a sampler
    void VulkanTextureResource::initSampler(Texture* tex)
    {
        float maxLOD = tex->m_mipmaps.size() == 1 ? 0.0f : (float)tex->m_mipmaps.size();
        sampler = std::unique_ptr<VulkanSampler>(new VulkanSampler(VulkanBase::getDevice(), tex->getSampler(), maxLOD));

        imageInfo.sampler = sampler->get();
    }


}
