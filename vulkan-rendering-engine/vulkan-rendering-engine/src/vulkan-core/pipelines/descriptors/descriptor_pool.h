#ifndef DESCRIPTOR_POOL_H_
#define DESCRIPTOR_POOL_H_

#include "build_options.h"
#include "descriptor_set_layout.h"
#include "descriptor_set.h"
#include <vector>

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  DescriptorPool class
    //---------------------------------------------------------------------------

    class DescriptorPool
    {
    public:
        DescriptorPool(VkDevice device, const std::vector<VkDescriptorPoolSize>& typeCounts, const uint32_t& maxSets);
        ~DescriptorPool() { vkDestroyDescriptorPool(device, descriptorPool, nullptr); };

        // Return the handle from the descriptor pool
        VkDescriptorPool& get() { return descriptorPool; }

        // Allocate one descriptor set from this pool.
        bool allocate(DescriptorSet* descriptorSet);

        // Allocate several descriptor sets from this pool.
        bool allocate(std::vector<DescriptorSet*> descriptorSets);

    private:
        // forbid copy and copy assignment
        DescriptorPool(const DescriptorPool& descriptorPool);
        DescriptorPool& operator=(const DescriptorPool& descriptorPool) {};

        // Handle for the VkDescriptorPool
        VkDescriptorPool descriptorPool;

        // Store a reference for destroying the vkDescriptorPool
        VkDevice device;

        // Number of sets, which can be allocated from this pool
        uint32_t maxSets;

        // Allocated Sets from this pool.
        uint32_t allocatedSets;
    };


}





#endif // !DESCRIPTOR_POOL_H_
