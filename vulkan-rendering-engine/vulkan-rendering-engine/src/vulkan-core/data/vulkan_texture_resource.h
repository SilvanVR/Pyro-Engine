#ifndef VULKAN_TEXTURE_RESOURCE
#define VULKAN_TEXTURE_RESOURCE

#include "vulkan_resource.hpp"
#include "vulkan-core/util_classes/vulkan_image.h"
#include "vulkan-core/util_classes/vulkan_other.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class Texture;

    //---------------------------------------------------------------------------
    //  VulkanTextureResource Class
    //---------------------------------------------------------------------------

    class VulkanTextureResource : public VulkanResource
    {
    public:
        // Load the texture data into gpu memory and initialize the VkDescriptorImageInfo struct for use in a descriptor set
        VulkanTextureResource(Texture* texture, void* data, uint32_t size);
        VulkanTextureResource(const VkDescriptorImageInfo& imageInfo);
        virtual ~VulkanTextureResource();

        // Return the descriptor image info, which can be used to update a descriptor set (used by the material)
        VkDescriptorImageInfo* getDescriptorImageInfo() { return &imageInfo; }

        // Needed for the shadow-renderer (access point-lights image)
        VulkanImage& getVulkanImage(){ return *image; }
        const std::shared_ptr<VulkanImageView>& getView(){ return view; }

        // Push the given data to the GPU using staging
        void push(const void* data, uint32_t size, uint32_t offset) { image->push(data, size, offset); }

    protected:
        // VkImage Handle + Memory
        std::unique_ptr<VulkanImage>        image;
        std::shared_ptr<VulkanImageView>    view;
        std::unique_ptr<VulkanSampler>      sampler;

        // Contains the sampler, a image-view and the image layout
        VkDescriptorImageInfo imageInfo;

        void loadTexDataIntoGPU(Texture* tex, void* data, uint32_t size);   // Load the texture data into memory
        void initTexture(Texture* tex, bool pushTexDataToGPU);
        void initSampler(Texture* tex);

    private:
        //forbid copy and copy assignment
        VulkanTextureResource(const VulkanTextureResource& vulkanUniformResource) = delete;
        VulkanTextureResource& operator=(const VulkanTextureResource& vulkanUniformResource) = delete;
    };

}

#endif // !VULKAN_TEXTURE_RESOURCE

