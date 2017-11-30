#include "descriptor_set_layout.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include <assert.h>
#include <algorithm> // std::sort

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    // Create a DescriptorSetLayout with several descriptors.
    DescriptorSetLayout::DescriptorSetLayout(VkDevice _device, const std::string& _name, const uint32_t _setNumber, const std::vector<DescriptorLayoutBinding>& _shaderBindings)
        : device(_device), name(_name), setNumber(_setNumber), shaderBindings(_shaderBindings)
    {
        createVkDescriptorSetLayout(false);
    }

    //---------------------------------------------------------------------------
    //  Public Functions
    //---------------------------------------------------------------------------

    // Return one shader-binding.
    const DescriptorLayoutBinding& DescriptorSetLayout::getBinding(const std::string& bindingName) const
    {
        for (unsigned int i = 0; i < shaderBindings.size(); i++)
            if (shaderBindings[i].name == bindingName)
                return shaderBindings[i];

        Logger::Log("DescriptorSetLayout::getBinding(): Given Binding-Name '" + bindingName 
                    + "' ist not present in '" + name + "'-Layout.", LOGTYPE_ERROR);
        return shaderBindings[0];
    }

    // Return the binding-number for a given binding name. Work aswell with all of the member-variables (e.g. names in a struct)
    uint32_t DescriptorSetLayout::getBindingNum(const std::string& bindingName) const
    {
        // Loop through every binding and search for the name
        for (unsigned int i = 0; i < shaderBindings.size(); i++)
        {
            // Check binding-name
            if (shaderBindings[i].name == bindingName)
                return i;

            // Check all member-name if the binding contains a struct
            for (const auto& mem : shaderBindings[i].bufferRanges)
                if (mem.name == bindingName)
                    return i;
        }

        Logger::Log("DescriptorSetLayout::getBindingNum(): Given Binding-Name '" + bindingName 
                    + "' ist not present in '" + name + "'-Layout.", LOGTYPE_ERROR);
        return -1;
    }

    // Return the offsets of a binding if its a struct.
    const std::vector<BufferRange>& DescriptorSetLayout::getBufferRanges(const std::string& bindingName)
    {
        // Loop through every binding and search for the name
        for (unsigned int i = 0; i < shaderBindings.size(); i++)
            if (shaderBindings[i].name == bindingName)
                return shaderBindings[i].bufferRanges;

        Logger::Log("DescriptorSetLayout::getBufferRanges(): Given Binding-Name '" + bindingName + "' ist not present in '" + name + "'-Layout.", LOGTYPE_ERROR);
        return shaderBindings[0].bufferRanges;
    }

    // Loop though every binding and search for a member with the name "bufferName". The Set-Name is also allowed which return the first buffer-range.
    const BufferRange& DescriptorSetLayout::getBufferRange(const std::string& bufferName)
    {
        for (unsigned int i = 0; i < shaderBindings.size(); i++)
        {
            for (const auto& bufferRange : shaderBindings[i].bufferRanges)
                if (bufferRange.name == bufferName)
                    return bufferRange;
        }

        // No buffer-range was found, so try to search for the set-name and return the first buffer-range from it
        for (unsigned int i = 0; i < shaderBindings.size(); i++)
            if (shaderBindings[i].name == bufferName)
                return shaderBindings[i].bufferRanges[0];

        Logger::Log("ERROR in DescriptorSetLayout::getBufferRange(): Given Buffer-Name '" + bufferName + "' ist not present in '" + name + "'-Layout."
            " Are the names in code and in the shader equal? Do you gave two different uniform-buffers the same binding-num by mistake? ", LOGTYPE_ERROR);
        return shaderBindings[0].bufferRanges[0];
    }

    // Merge two descriptor-set-layouts from different shader-stages together.
    void DescriptorSetLayout::mergeDescriptorSetLayout(DescriptorSetLayout* otherSetLayout)
    {
        if (otherSetLayout->isMaterialSet())
            m_isMaterialSet = true;
        if (otherSetLayout->isShaderSet())
            m_isShaderSet = true;

        // Merge the two set-layouts together. Add shader-bindings to the first one from the second one if not present and add ShaderStage-access mask.
        std::vector<DescriptorLayoutBinding> otherShaderBindings;

        // Loop through every binding from the second-layout and check if its present in this layout. If not it must be added, otherwise just add the shader-stage.
        for (const auto& otherShaderBinding : otherSetLayout->getBindings())
        {
            bool found = false;

            for (auto& binding : getBindings())
            {
                if (binding == otherShaderBinding)// same binding was found the same as the other on, so simply add the shader-stage to it
                {
                    binding.shaderStage |= otherShaderBinding.shaderStage;
                    found = true;
                    break;
                }
            }

            if (!found) // binding wasn't found so it must be unique to the other shader-stage so add it to this layout bindings
                shaderBindings.push_back(otherShaderBinding);
        }

        // Sort all bindings by binding-num
        std::sort(shaderBindings.begin(), shaderBindings.end(), [](const DescriptorLayoutBinding& binding1, const DescriptorLayoutBinding& binding2) -> bool
        { return binding1.bindingNum < binding2.bindingNum; });

        createVkDescriptorSetLayout(true);
    }

    // Loop through all bindings + buffernames and return the appropriate data-type
    DataType DescriptorSetLayout::getDataType(const std::string& name)
    {
        for (const auto& binding : shaderBindings)
        {
            // Search for an data-field with the name in all uniform structs
            for (const auto& bufferRange : binding.bufferRanges)
                if (bufferRange.name == name)
                    return bufferRange.dataType;
        }

        for (const auto& binding : shaderBindings)
        {
            if (binding.name == name)
                if (binding.dataType != DataType::UNKNOWN || binding.dataType != DataType::Struct)
                    return binding.dataType;
        }

        Logger::Log("DescriptorSetLayout::getDataType(): Field #" + name + " is not present in descriptor-set-layout #"
                    + getName(), LOGTYPE_WARNING);
        return DataType::UNKNOWN;
    }

    //---------------------------------------------------------------------------
    //  Private Functions
    //---------------------------------------------------------------------------

    void DescriptorSetLayout::createVkDescriptorSetLayout(bool destroyOldOne)
    {
        if (destroyOldOne)
            vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        for (unsigned int i = 0; i < shaderBindings.size(); i++)
        {
            // Binding i : types[i] (shaderStages[i])
            VkDescriptorSetLayoutBinding layoutBinding;
            layoutBinding.binding               = shaderBindings[i].bindingNum;
            layoutBinding.descriptorType        = shaderBindings[i].type;
            layoutBinding.descriptorCount       = shaderBindings[i].descriptorCount;
            layoutBinding.stageFlags            = shaderBindings[i].shaderStage;
            layoutBinding.pImmutableSamplers    = nullptr;

            layoutBindings.push_back(layoutBinding);
        }

        /* Create a descriptor layout for use resources in shaders */
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = nullptr;
        layoutInfo.flags = 0;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);

        precalculateHashCode();
    }

    void DescriptorSetLayout::precalculateHashCode()
    {
        static const int BASE = 17;
        static const int MULTIPLIER = 31;

        int result = BASE;

        for (auto& binding : shaderBindings)
            result = MULTIPLIER * result + binding.type;

        for (auto& letter : this->name)
            result = MULTIPLIER * result + letter;

        m_hashCode = result;
    }


}