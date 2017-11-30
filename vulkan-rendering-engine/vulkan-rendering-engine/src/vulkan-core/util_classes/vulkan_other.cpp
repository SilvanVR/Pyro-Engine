#include "vulkan_other.h"

#include "vulkan-core/data/material/texture/sampler.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  VulkanSemaphore - Class
    //---------------------------------------------------------------------------

    VulkanSemaphore::VulkanSemaphore(VkDevice _device)
        : device(_device)
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = nullptr;
        semaphoreInfo.flags = 0;

        VkResult res = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &sem);
        assert(res == VK_SUCCESS);
    }

    VulkanSemaphore::~VulkanSemaphore()
    {
        vkDestroySemaphore(device, sem, nullptr);
    }

    //---------------------------------------------------------------------------
    //  VulkanFence - Class
    //---------------------------------------------------------------------------

    VulkanFence::VulkanFence(VkDevice _device, const VkFenceCreateFlags& flags)
        : device(_device)
    {
        VkFenceCreateInfo fenceInfo;
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.pNext = nullptr;
        fenceInfo.flags = flags;

        VkResult res = vkCreateFence(device, &fenceInfo, nullptr, &fence);
        assert(res == VK_SUCCESS);
    }

    VulkanFence::~VulkanFence()
    {
        vkDestroyFence(device, fence, nullptr);
    }

    void VulkanFence::wait(const uint64_t& waitTime)
    {
        VkResult res;
        do {
            res = vkWaitForFences(device, 1, &fence, VK_TRUE, waitTime);
        } while (res == VK_TIMEOUT);
        assert(res == VK_SUCCESS);
    }

    void VulkanFence::reset()
    {
        vkResetFences(device, 1, &fence);
    }

    void VulkanFence::wait(const std::vector<const VulkanFence*> fences, const VkBool32& waitAll, const uint64_t& waitTime)
    {
        std::vector<VkFence> vkFences;

        for(const auto& fence : fences)
            vkFences.push_back(fence->get());

        VkResult res;
        do {
            res = vkWaitForFences(fences[0]->device, static_cast<uint32_t>(vkFences.size()), vkFences.data(), waitAll, waitTime);
        } while (res == VK_TIMEOUT);
        assert(res == VK_SUCCESS);
    }

    //---------------------------------------------------------------------------
    //  VulkanSampler - Class
    //---------------------------------------------------------------------------

    VulkanSampler::VulkanSampler(VkDevice _device, const std::shared_ptr<Sampler>& sampler, float maxLOD)
        : device(_device)
    {
        VkFilter minFilter;
        VkFilter magFilter;
        VkSamplerMipmapMode mipmapMode;
        VkSamplerAddressMode addressMode;

        convertToVulkanSampler(sampler, minFilter, magFilter, mipmapMode, addressMode);

        initVkSampler(minFilter, magFilter, mipmapMode, addressMode, sampler->getMaxAnisotropy(), maxLOD);
    }

    VulkanSampler::VulkanSampler(VkDevice _device, const VkSamplerCreateInfo& createInfo)
        : device(_device)
    {
        initVkSampler(createInfo);
    }

    VulkanSampler::VulkanSampler(VkDevice _device, const VkFilter& filter, const VkSamplerAddressMode& addressMode)
        : device(_device)
    {
        initVkSampler(filter, filter, VK_SAMPLER_MIPMAP_MODE_NEAREST, addressMode, 1.0f, 0.0f);
    }

    VulkanSampler::~VulkanSampler()
    {
        vkDeviceWaitIdle(device);
        vkDestroySampler(device, sampler, nullptr);
    }

    void VulkanSampler::initVkSampler(const VkFilter& minFilter, const VkFilter& magFilter, const VkSamplerMipmapMode& mipmapMode,
                                      const VkSamplerAddressMode& addressMode, float maxAnisotropy, float maxLOD)
    {
        assert(maxAnisotropy != 0.0f);

        VkSamplerCreateInfo samplerCreateInfo = {};
        samplerCreateInfo.sType             = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.pNext             = nullptr;
        samplerCreateInfo.flags             = 0;
        samplerCreateInfo.magFilter         = minFilter;
        samplerCreateInfo.minFilter         = magFilter;
        samplerCreateInfo.mipmapMode        = mipmapMode;
        samplerCreateInfo.addressModeU      = addressMode;
        samplerCreateInfo.addressModeV      = addressMode;
        samplerCreateInfo.addressModeW      = addressMode;
        samplerCreateInfo.mipLodBias        = 0.0;
        samplerCreateInfo.anisotropyEnable  = maxAnisotropy == 1.0f ? VK_FALSE : VK_TRUE;
        samplerCreateInfo.maxAnisotropy     = maxAnisotropy;
        samplerCreateInfo.compareEnable     = VK_FALSE;
        samplerCreateInfo.compareOp         = VK_COMPARE_OP_NEVER;
        samplerCreateInfo.minLod            = 0.0f;
        samplerCreateInfo.maxLod            = maxLOD;
        samplerCreateInfo.borderColor       = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

        initVkSampler(samplerCreateInfo);
    }

    void VulkanSampler::initVkSampler(const VkSamplerCreateInfo& createInfo)
    {
        // create the sampler 
        VkResult res = vkCreateSampler(device, &createInfo, nullptr, &sampler);
        assert(res == VK_SUCCESS);
    }


    void VulkanSampler::convertToVulkanSampler(const std::shared_ptr<Sampler>& sampler,
        VkFilter& minFilter, VkFilter& magFilter, VkSamplerMipmapMode& mipmapMode, VkSamplerAddressMode& addressMode)
    {
        switch (sampler->getMinFilter())
        {
        case FILTER_LINEAR:     minFilter = VK_FILTER_LINEAR; break;
        case FILTER_NEAREST:    minFilter = VK_FILTER_NEAREST; break;
        }

        switch (sampler->getMagFilter())
        {
        case FILTER_LINEAR:     magFilter = VK_FILTER_LINEAR; break;
        case FILTER_NEAREST:    magFilter = VK_FILTER_NEAREST; break;
        }

        switch (sampler->getMipmapMode())
        {
        case MIPMAP_MODE_LINEAR:    mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; break;
        case MIPMAP_MODE_NEAREST:   mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST; break;
        }

        switch (sampler->getAddressMode())
        {
        case ADDRESS_MODE_CLAMP_TO_EDGE:    addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; break;
        case ADDRESS_MODE_REPEAT:           addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT; break;
        case ADDRESS_MODE_MIRRORED_REPEAT:  addressMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; break;
        case ADDRESS_MODE_CLAMP_TO_BORDER:  addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; break;
        }
    }

}