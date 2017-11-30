#include "descriptor_pool_manager.h"

#include "vulkan-core/vkTools/vk_tools.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Allocates a descriptor-set from a name, which represents a set-layout.
    DescriptorSet* DescriptorPoolManager::allocateSet(DescriptorSetLayout* setLayout)
    {
        DescriptorSet* newDescriptorSet = new DescriptorSet(device, setLayout);

        // Get the pool for a given set-layout and allocate a set from it.
        if (!getPool(setLayout)->allocate(newDescriptorSet))
        {
            // Pool-Allocation wasnt successful, so create a new pool and allocate from it.
            createPool(setLayout)->allocate(newDescriptorSet);
        }

        // Create for every uniform-binding a VkBuffer (and allocate device mem)
        newDescriptorSet->createUniformBuffers();

        return newDescriptorSet;
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Algorithm:
    // 1. Check if an descriptor-pool exists for this set-layout
    // 1.1 YES: Allocate Descriptor-Set. IF NOT successful, create a new descriptor-pool for that set-layout and allocate further on from that new pool.
    // 1.2 NO: Create a new descriptor-pool for that set-layout.
    DescriptorPool* DescriptorPoolManager::getPool(DescriptorSetLayout* setLayout)
    {
        for (auto& iterator = descriptorPoolHashMap.begin(); iterator != descriptorPoolHashMap.end(); iterator++)
        {
            // A pool was found. Return last created pool for that layout.
            if (setLayout->hashCode() == iterator->first)
                return iterator->second.back();
        }

        // No Pool was found, create one and return it.
        return createPool(setLayout);
    }

    // Create a new pool compatible with the given set-layout.
    DescriptorPool* DescriptorPoolManager::createPool(DescriptorSetLayout* setLayout)
    {
        const std::vector<DescriptorLayoutBinding>& bindings = setLayout->getBindings();

        std::vector<VkDescriptorPoolSize> typeCounts;

        // Set descriptor-count for every binding type based on the ENUM-VALUES
        for (auto& binding : bindings)
        {
            VkDescriptorPoolSize typeCount;
            typeCount.type = binding.type;
            typeCount.descriptorCount = MAX_SETS * binding.descriptorCount;

            typeCounts.push_back(typeCount);
        }

        DescriptorPool* descriptorPool = new DescriptorPool(device, typeCounts, MAX_SETS);

        // Add the pool to the vector of pools for that layout.
        descriptorPoolHashMap[setLayout->hashCode()].push_back(descriptorPool);

        return descriptorPool;
    }

}