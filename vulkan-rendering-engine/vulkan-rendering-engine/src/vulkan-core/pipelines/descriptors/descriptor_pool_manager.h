#ifndef DESCRIPTOR_POOL_MANAGER_H_
#define DESCRIPTOR_POOL_MANAGER_H_

#include "descriptor_pool.h"

#include <map>

namespace Pyro
{


    class DescriptorPoolManager
    {
        // Max. Descriptor-Sets from one Pool. Pool Manager creates a new pool for a set-layout if number of allocations exceeds this value.
        const int MAX_SETS = 1024;

        // Contains all Descriptor-Pools. Assign the hash-code from a set-layout to a vector of pools.
        std::map<int, std::vector<DescriptorPool*>> descriptorPoolHashMap;

    public:
        DescriptorPoolManager(VkDevice _device)
            : device(_device)
        {}

        ~DescriptorPoolManager()
        {
            for (auto& iterator = descriptorPoolHashMap.begin(); iterator != descriptorPoolHashMap.end(); iterator++)
                for (auto& pool : iterator->second)
                    delete pool;
        }

        // Allocates a descriptor-set from a name, which represents a set-layout.
        DescriptorSet* allocateSet(DescriptorSetLayout* setLayout);

    private:
        // Reference needed for creating new pools.
        VkDevice device;

        // Algorithm:
        // 1. Check if an descriptor-pool exists for this set-layout
        // 1.1 YES: Allocate Descriptor-Set. IF NOT successful, create a new descriptor-pool for that set-layout and allocate further on from that new pool.
        // 1.2 NO: Create a new descriptor-pool for that set-layout.
        DescriptorPool* getPool(DescriptorSetLayout* setLayout);

        // Create a new pool compatible with the given set-layout and return it.
        DescriptorPool* createPool(DescriptorSetLayout* setLayout);
    };


}

#endif // !DESCRIPTOR_POOL_MANAGER_H_
