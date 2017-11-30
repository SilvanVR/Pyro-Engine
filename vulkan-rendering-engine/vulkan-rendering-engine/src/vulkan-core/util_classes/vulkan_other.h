#ifndef VULKAN_OTHER_H_
#define VULKAN_OTHER_H_


// Contains several small classes which encapsulates an corresponding Vk-Object

#include "build_options.h"

namespace Pyro
{

    class Sampler;

    //---------------------------------------------------------------------------
    //  VulkanSemaphore - Class
    //---------------------------------------------------------------------------

    class VulkanSemaphore
    {
        friend class CommandBuffer;
        friend class Swapchain;

    public:
        VulkanSemaphore(VkDevice device);
        ~VulkanSemaphore();

        const VkSemaphore& get() const { return sem; }

    private:
        VkDevice device;
        VkSemaphore sem;
    };

    //---------------------------------------------------------------------------
    //  VulkanFence - Class
    //---------------------------------------------------------------------------

    class VulkanFence
    {
        friend class CommandBuffer;
        friend class Swapchain;

    public:
        VulkanFence(VkDevice device, const VkFenceCreateFlags& flags = 0);
        ~VulkanFence();

        const VkFence& get() const { return fence; }
        void wait(const uint64_t& waitTime = UINT64_MAX);
        void reset();

        static void wait(const std::vector<const VulkanFence*> fences, const VkBool32& waitAll, const uint64_t& waitTime = UINT64_MAX);

    private:
        VkDevice    device;
        VkFence     fence;
    };

    //---------------------------------------------------------------------------
    //  VulkanSampler - Class
    //---------------------------------------------------------------------------

    class VulkanSampler
    {
    public:
        VulkanSampler(VkDevice device, const std::shared_ptr<Sampler>& sampler, float maxLOD);
        VulkanSampler(VkDevice device, const VkSamplerCreateInfo& createInfo);
        VulkanSampler(VkDevice device, const VkFilter& filter, const VkSamplerAddressMode& addressMode);
        ~VulkanSampler();

        const VkSampler& get() const { return sampler; }

    private:
        VkDevice    device;
        VkSampler   sampler;

        // Create the VkSampler
        void initVkSampler(const VkFilter& minFilter, const VkFilter& magFilter, const VkSamplerMipmapMode& mipmapMode, 
                           const VkSamplerAddressMode& addressMode, float maxAnisotropy, float maxLOD);
        void initVkSampler(const VkSamplerCreateInfo& createInfo);

        void convertToVulkanSampler(const std::shared_ptr<Sampler>& sampler, 
              VkFilter& minFilter, VkFilter& magFilter, VkSamplerMipmapMode& mipmapMode, VkSamplerAddressMode& addressMode);
    };


}




#endif // !VULKAN_OTHER_H_

