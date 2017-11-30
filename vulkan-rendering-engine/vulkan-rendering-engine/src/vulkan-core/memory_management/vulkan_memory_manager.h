#ifndef VULKAN_MEMORY_MANAGER_H_
#define VULKAN_MEMORY_MANAGER_H_

#include "vulkan-core/pipelines/descriptors/descriptor_pool_manager.h"

#include <map>
#include <string>

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class VulkanBase;
    class Texture;
    class Mesh;

    //---------------------------------------------------------------------------
    //  Structs
    //---------------------------------------------------------------------------

    struct GPUMemoryInfo
    {
        float    percentageUsed;
        uint64_t totalMemory;
        uint64_t currentAllocated;
        uint64_t totalAllocated;
        uint64_t totalFreed;
        uint64_t totalAllocations;
        uint64_t totalDeallocations;

        void allocated(uint64_t size)
        {
            currentAllocated += size;
            totalAllocated += size;
            percentageUsed = (currentAllocated / (float)totalMemory) * 100.0f;
            totalAllocations++;
        }

        void deallocated(uint64_t size)
        {
            totalFreed += size;
            currentAllocated -= size;
            percentageUsed = (currentAllocated / (float)totalMemory) * 100.0f;
            totalDeallocations++;
        }

        GPUMemoryInfo() : percentageUsed(0), currentAllocated(0), totalAllocated(0),
                          totalFreed(0), totalAllocations(0), totalDeallocations(0) {}
    };

    //---------------------------------------------------------------------------
    //  VMM class
    //---------------------------------------------------------------------------

    class VMM
    {
        VulkanBase* vulkanBase;

    private:
        // Handles the allocation of new Descriptor-Sets from Descriptor-Pools
        DescriptorPoolManager descriptorPoolManager;

        // Assign a string to a Descriptor-Set-Layout (e.g. ViewProjection <-> 1 Uniform-Buffer). Contains all Descriptor-Set-Layouts used in the program.
        // Allows the creation of a descriptor-set with a string.
        std::map<std::string, DescriptorSetLayout*> programDescriptorSetLayouts;

        // Store the information about how much device memory has been allocated for each request
        std::map<VkDeviceMemory, VkDeviceSize> memorySizeMap;

        // Track all kind of memory information from the GPU
        GPUMemoryInfo memoryInfo;

        // Static instance, to call functions in a static way.
        static VMM* INSTANCE;

    public:
        VMM(VulkanBase* vulkanBase);
        ~VMM();

        static const GPUMemoryInfo& getMemoryInfo(){ return INSTANCE->memoryInfo; }
        static void log();

        //---------------------------------------------------------------------------
        //  Allocate Functions
        //---------------------------------------------------------------------------

        static VkDeviceMemory allocateMemory(const VkMemoryRequirements& memReqs, const VkFlags& requirementsMask);
        static void freeMemory(const VkDeviceMemory& mem);

        //---------------------------------------------------------------------------
        //  Others
        //---------------------------------------------------------------------------

        // Add a Descriptor-Set-Layout to the map of all set-layouts. Called in the shader class.
        static void addDescriptorSetLayout(const std::string& setName, DescriptorSetLayout* setLayout);

        //---------------------------------------------------------------------------
        //  Getter's
        //---------------------------------------------------------------------------

        static DescriptorSetLayout* getSetLayout(const std::string& name){ return INSTANCE->programDescriptorSetLayouts[name]; }

        //---------------------------------------------------------------------------
        //  Create Descriptor Sets
        //---------------------------------------------------------------------------

        static DescriptorSet* createDescriptorSet(const std::string& setName);
        static DescriptorSet* createDescriptorSet(DescriptorSetLayout* setLayout);

    };

}

#endif // !VULKAN_MEMORY_MANAGER_H_
