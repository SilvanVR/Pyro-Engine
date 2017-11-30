#include "descriptor_pool.h"

#include <assert.h>

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------

    DescriptorPool::DescriptorPool(VkDevice _device, const std::vector<VkDescriptorPoolSize>& typeCounts, const uint32_t& _maxSets)
        : device(_device), maxSets(_maxSets), allocatedSets(0)
    {
        // Create the descriptor pool
        VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.pNext = nullptr;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(typeCounts.size());
        descriptorPoolInfo.pPoolSizes = typeCounts.data();
        descriptorPoolInfo.maxSets = maxSets;

        VkResult res = vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool);
        assert(res == VK_SUCCESS);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Allocate one descriptor set from this pool
    bool DescriptorPool::allocate(DescriptorSet* descriptorSet)
    {
        if ((allocatedSets + 1) > maxSets)
            return false;

        VkDescriptorSetAllocateInfo allocInfo[1];
        allocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo[0].pNext = nullptr;
        allocInfo[0].descriptorPool = descriptorPool;
        allocInfo[0].descriptorSetCount = 1;
        allocInfo[0].pSetLayouts = &descriptorSet->getSetLayout()->get();

        VkResult res = vkAllocateDescriptorSets(device, allocInfo, &descriptorSet->get());
        assert(!res);

        allocatedSets++;
        return true;
    }


    // Allocate several descriptor sets from this pool all using the same layout.
    bool DescriptorPool::allocate(std::vector<DescriptorSet*> descriptorSets)
    {
        if ((allocatedSets + descriptorSets.size()) > maxSets)
            return false;

        std::vector<VkDescriptorSetLayout>  vkSetLayouts;
        std::vector<VkDescriptorSet>        vkDescriptorSets;

        for (unsigned int i = 0; i < descriptorSets.size(); i++)
            vkSetLayouts.push_back(descriptorSets[i]->getSetLayout()->get());

        for (auto& set : descriptorSets)
            vkDescriptorSets.push_back(set->get());

        VkDescriptorSetAllocateInfo allocInfo[1];
        allocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo[0].pNext = nullptr;
        allocInfo[0].descriptorPool = descriptorPool;
        allocInfo[0].descriptorSetCount = static_cast<uint32_t>(vkSetLayouts.size());
        allocInfo[0].pSetLayouts = vkSetLayouts.data();

        VkResult res = vkAllocateDescriptorSets(device, allocInfo, vkDescriptorSets.data());
        assert(!res);

        allocatedSets += static_cast<uint32_t>(descriptorSets.size());
        return true;
    }

}
