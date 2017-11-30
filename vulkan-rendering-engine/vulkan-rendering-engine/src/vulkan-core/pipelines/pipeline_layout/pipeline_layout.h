/*
*  PipelineLayout-Class header file.
*  Encapsulates a VkPipelineLayout.
*  Creates a pipeline layout based on the given arguments in the constructor.
*
*  Date:    25.04.2016
*  Creator: Silvan Hau
*/

#ifndef PIPELINE_LAYOUT_H_
#define PIPELINE_LAYOUT_H_

#include "build_options.h"
#include <vector>
#include <map>

namespace Pyro
{


    class DescriptorSetLayout;

    //---------------------------------------------------------------------------
    //  PipelineLayout class
    //---------------------------------------------------------------------------

    class PipelineLayout
    {

    public:
        // Creates a VkPipelineLayout with no descriptor Sets
        PipelineLayout(VkDevice device);

        // Creates a VkPipelineLayout with one descriptor Set
        PipelineLayout(VkDevice device, DescriptorSetLayout* setLayout);

        // Creates a VkPipelineLayout with several descriptor sets
        PipelineLayout(VkDevice device, const std::vector<DescriptorSetLayout*>& setLayouts);

        // Creates a VkPipelineLayout with several descriptor sets and several push constants
        PipelineLayout(VkDevice device, const std::vector<DescriptorSetLayout*>& setLayouts, const std::vector<VkPushConstantRange>& pushConstants);

        // Destructor *clean up VkPipelineLayout*
        ~PipelineLayout() { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); }

        // Return the VkPipelineLayout
        VkPipelineLayout& get() { return pipelineLayout; }

        VkShaderStageFlags getPushConstantShaderStage() const { return pushConstantShaderStage; }

        // Find the Number where a descriptor-set should be bind in this pipeline-layout. Assert if it doesnt exists.
        uint32_t getSetNumber(DescriptorSetLayout* setLayout);

        // Return all set-layout used in this pipeline-layout
        std::vector<DescriptorSetLayout*>& getSetLayouts() { return descriptorSetLayouts; }

        // Return one descriptor-set-layout using the binding-num. No Boundings check.
        DescriptorSetLayout* getSetLayout(uint32_t dstBinding) { return descriptorSetLayouts[dstBinding]; }

    private:
        //forbid copy and copy assignment
        PipelineLayout(const PipelineLayout& pipelineLayout) {};
        PipelineLayout& operator=(const PipelineLayout& pipelineLayout) {};

        // The VkPipelineLayout
        VkPipelineLayout pipelineLayout;

        // Need a reference to the logical device for destruction 
        VkDevice device;

        // The DescriptorSetLayouts present in this PipelineLayout
        std::vector<DescriptorSetLayout*> descriptorSetLayouts;

        // Assign a set-layout to the binding-number in this pipeline-layout
        std::map<int, uint32_t> setLayoutIndexHashMap;

        // Where the push-constant is available in the pipeline
        VkShaderStageFlags pushConstantShaderStage;
    };

}

#endif // !PIPELINE_LAYOUT_H_
