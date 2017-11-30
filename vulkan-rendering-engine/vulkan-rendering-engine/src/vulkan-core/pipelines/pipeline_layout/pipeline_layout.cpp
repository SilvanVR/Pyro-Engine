
#include "pipeline_layout.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "vulkan-core/pipelines/descriptors/descriptor_set_layout.h"
#include <assert.h>

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------

    // Creates a VkPipelineLayout with no descriptor Sets
    PipelineLayout::PipelineLayout(VkDevice _device)
        : PipelineLayout(device, {}, {})
    {}

    // Creates a VkPipelineLayout with one descriptor Set
    PipelineLayout::PipelineLayout(VkDevice _device, DescriptorSetLayout* setLayout)
        : PipelineLayout(device, { setLayout }, {})
    {}

    // Creates a VkPipelineLayout with several descriptor sets, storing the names from the layouts to find a setNumber for a descriptor set.
    PipelineLayout::PipelineLayout(VkDevice _device, const std::vector<DescriptorSetLayout*>& setLayouts)
        : PipelineLayout(device, { setLayouts }, {})
    {}

    // Creates a VkPipelineLayout with several descriptor sets and several push constants
    PipelineLayout::PipelineLayout(VkDevice _device, const std::vector<DescriptorSetLayout*>& setLayouts, const std::vector<VkPushConstantRange>& pushConstants)
        : device(_device), descriptorSetLayouts(setLayouts)
    {
        // Get the "Vulkan" Descriptor-Set-Layouts
        std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
        for (unsigned int i = 0; i < setLayouts.size(); i++)
        {
            vkDescriptorSetLayouts.push_back(setLayouts[i]->get());

            // Calculate hashCode
            int hashCode = setLayouts[i]->hashCode();

            // Check if such an set already exists.
            assert(setLayoutIndexHashMap.count(hashCode) == 0 && "ERROR in PipelineLayout::PipelineLayout(): Duplicated Set-Layout! "
                                                                 "Hashcode of two sets in a Shader were equal.");

            // Save the index of the layout
            setLayoutIndexHashMap[hashCode] = i;
        }


        for (auto& pushConstant : pushConstants)
            pushConstantShaderStage |= pushConstant.stageFlags;

        VkPipelineLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = nullptr;
        layoutInfo.flags = 0;
        layoutInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
        layoutInfo.pSetLayouts = vkDescriptorSetLayouts.data();
        layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        layoutInfo.pPushConstantRanges = pushConstants.data();

        VkResult res = vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout);
        assert(res == VK_SUCCESS);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Find the Number where a descriptor-set should be bind in this pipeline-layout. Assertion if it doesnt exists.
    uint32_t PipelineLayout::getSetNumber(DescriptorSetLayout* setLayout)
    {
        int hashCode = setLayout->hashCode();
        assert(setLayoutIndexHashMap.count(hashCode) > 0 && "ERROR in PipelineLayout::getSetNumber(): Given set-layout is not present in this pipeline-layout.");
        return setLayoutIndexHashMap[hashCode];
    }

}
