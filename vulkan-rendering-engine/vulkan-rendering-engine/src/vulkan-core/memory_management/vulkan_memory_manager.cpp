#include "vulkan_memory_manager.h"

#include "memory_manager/memory_manager.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "vulkan-core/vulkan_base.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Static Fields
    //---------------------------------------------------------------------------

    VMM* VMM::INSTANCE;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    VMM::VMM(VulkanBase* _vulkanBase)
        : vulkanBase(_vulkanBase), descriptorPoolManager(_vulkanBase->getDevice())
    {
        if (INSTANCE == nullptr)
            INSTANCE = this;
        else
            Logger::Log("VMM::VMM(): Could not create a second Vulkan-Memory-Manager. That is not allowed!", LOGTYPE_ERROR);

        memoryInfo.totalMemory = vulkanBase->getGPU().memoryProperties.memoryHeaps[0].size;
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    VMM::~VMM()
    {}

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void VMM::log()
    {
        Logger::Log("------------ GPU MEMORY INFO -------------", LOGTYPE_INFO);
        Logger::Log("Total Memory: " + MemoryManager::bytesToString(getMemoryInfo().totalMemory), LOGTYPE_INFO);
        Logger::Log("Current Allocated: " + MemoryManager::bytesToString(getMemoryInfo().currentAllocated), LOGTYPE_INFO);
        Logger::Log("Percentage Used: " + TS(getMemoryInfo().percentageUsed) + " %", LOGTYPE_INFO);
        Logger::Log("Total Allocated: " + MemoryManager::bytesToString(getMemoryInfo().totalAllocated), LOGTYPE_INFO);
        Logger::Log("Total Freed: " + MemoryManager::bytesToString(getMemoryInfo().totalFreed), LOGTYPE_INFO);
        Logger::Log("Total Allocations: " + TS(getMemoryInfo().totalAllocations), LOGTYPE_INFO);
        Logger::Log("Total Deallocations: " + TS(getMemoryInfo().totalDeallocations), LOGTYPE_INFO);
        Logger::Log("------------------------------------------", LOGTYPE_INFO);
    }

    VkDeviceMemory VMM::allocateMemory(const VkMemoryRequirements& memReqs, const VkFlags& requirementsMask)
    {
        VkMemoryAllocateInfo allocInfo;
        allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext             = nullptr;
        allocInfo.allocationSize    = memReqs.size;
        vkTools::getMemoryType(memReqs.memoryTypeBits, requirementsMask, &allocInfo.memoryTypeIndex);

        VkDeviceMemory memory;
        VkResult err = vkAllocateMemory(VulkanBase::getDevice(), &allocInfo, nullptr, &memory);
        assert(!err);

        INSTANCE->memorySizeMap[memory] = allocInfo.allocationSize;
        INSTANCE->memoryInfo.allocated(allocInfo.allocationSize);

        return memory;
    }

    void VMM::freeMemory(const VkDeviceMemory& mem)
    {
        VkDeviceSize size = INSTANCE->memorySizeMap[mem];
        INSTANCE->memorySizeMap.erase(mem);
        INSTANCE->memoryInfo.deallocated(size);

        vkFreeMemory(VulkanBase::getDevice(), mem, nullptr);
    }

    // Add a Descriptor-Set-Layout to the map of all set-layouts. Called in the shader class.
    void VMM::addDescriptorSetLayout(const std::string& setName, DescriptorSetLayout* setLayout)
    {
        if (INSTANCE->programDescriptorSetLayouts.count(setName) == 0)
            INSTANCE->programDescriptorSetLayouts[setName] = setLayout;
        else
            Logger::Log("VMM::addDescriptorSetLayout(): Given setName '" + setName + "' is already present!", 
                         LOGTYPE_WARNING, LOG_LEVEL_NOT_IMPORTANT);
    }

    //---------------------------------------------------------------------------
    //  Create Uniform Buffers
    //---------------------------------------------------------------------------

    DescriptorSet* VMM::createDescriptorSet(const std::string& setName)
    {
        return INSTANCE->descriptorPoolManager.allocateSet(INSTANCE->programDescriptorSetLayouts[setName]);
    }

    DescriptorSet* VMM::createDescriptorSet(DescriptorSetLayout* setLayout)
    {
        return INSTANCE->descriptorPoolManager.allocateSet(setLayout);
    }


}
