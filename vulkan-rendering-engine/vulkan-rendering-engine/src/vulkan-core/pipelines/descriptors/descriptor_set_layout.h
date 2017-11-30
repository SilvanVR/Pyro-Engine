#ifndef DESCRIPTOR_SET_LAYOUT_H_
#define DESCRIPTOR_SET_LAYOUT_H_

#include "build_options.h"

namespace Pyro
{

    // Information about every data-member in a struct in a UBO
    struct BufferRange
    {
        std::string name;
        uint32_t    offset;
        uint32_t    range;
        uint32_t    index;
        DataType    dataType;
    };

    struct DescriptorLayoutBinding
    {
        VkDescriptorType         type;            // Descriptor-Type (e.g. uniform-buffer, image-sampler etc.)
        uint32_t                 descriptorCount; // greater 1: Array
        VkShaderStageFlags       shaderStage;     // Shader-Stage of this binding
        std::string              name;            // Binding name
        VkDeviceSize             bufferSize;      // Total buffer-size for this binding
        std::vector<BufferRange> bufferRanges;    // Buffer Information for every data-member
        uint32_t                 bindingNum;      // Binding slot
        DataType                 dataType;        // if this binding is a sampler this might be Texture2D or TextureCube otherwise its unknown

        bool operator==(const DescriptorLayoutBinding& other) 
        { return type == other.type && descriptorCount == other.descriptorCount && bindingNum == other.bindingNum; }
    };

    struct PushConstant
    {
        VkPushConstantRange pushConstantRange;
        std::string         name;
    };

    //---------------------------------------------------------------------------
    //  DescriptorSetLayout class
    //---------------------------------------------------------------------------

    class DescriptorSetLayout
    {
        friend class ShaderModule;  // Alow the shader-module class to set private fields like m_isMaterialSet
        friend class Shader;        // Allow the Shader-class to set the name

    public:
        // Create a DescriptorSetLayout with several descriptors.
        DescriptorSetLayout(VkDevice device, const std::string& name, const uint32_t setNumber, 
                            const std::vector<DescriptorLayoutBinding>& shaderBindings);
        ~DescriptorSetLayout() { vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr); }

        // Simple Getter's
        const VkDescriptorSetLayout& get() const { return descriptorSetLayout; }
        const std::string& getName() const { return name; }
        uint32_t getSetNumber() const { return setNumber; }
        uint32_t getBindingNum(const std::string& bindingName) const;

        // Return all shader bindings in this layout
        std::vector<DescriptorLayoutBinding>& getBindings() { return shaderBindings; }

        // Return one shader-binding. No Bounding check.
        const DescriptorLayoutBinding& getBinding(uint32_t index) const { return shaderBindings[index]; }

        // Return one shader-binding.
        const DescriptorLayoutBinding& getBinding(const std::string& bindingName) const;

        // Return the offsets of a binding if its a struct.
        const std::vector<BufferRange>& getBufferRanges(const std::string& bindingName);

        // Loop though every binding and search for a member with the name "bufferName".
        const BufferRange& getBufferRange(const std::string& bufferName);

        // Loop through all bindings + buffernames and return the appropriate data-type
        DataType getDataType(const std::string& name);

        // Return the shader stage for this set. Every binding MUST have the same shader-stage.
        VkShaderStageFlags getShaderStage() { return shaderBindings[0].shaderStage; }

        // Merge two descriptor-set-layouts from different shader-stages together.
        void mergeDescriptorSetLayout(DescriptorSetLayout* otherSetLayout);

        // Returns a hashCode for this set-layout to compare two sets with equal bindings
        int hashCode() { return m_hashCode; }

        // True if this set is a material-set
        bool isMaterialSet() { return m_isMaterialSet; }

        // True if this set is a shader-set
        bool isShaderSet() { return m_isShaderSet; }

    private:
        //forbid copy and copy assignment
        DescriptorSetLayout(const DescriptorSetLayout& descriptorSetLayout) {};
        DescriptorSetLayout& operator=(const DescriptorSetLayout& descriptorSetLayout) {};

        VkDevice                                device;                 // Reference for destruction
        VkDescriptorSetLayout                   descriptorSetLayout;    // Vulkan Descriptor-Set-Layout Handle
        std::vector<DescriptorLayoutBinding>    shaderBindings;         // Information about every binding
        std::string                             name;                   // Identifier
        uint32_t                                setNumber;              // Set-Number in Shader-Code
        bool                                    m_isMaterialSet;        // True if a material set
        bool                                    m_isShaderSet;          // True if a shader-set
        int                                     m_hashCode;             // Cached hashcode

        void precalculateHashCode();

        // Creates the VkDescriptorSetLayout.
        void createVkDescriptorSetLayout(bool destroyOldOne = false);
    };



}


#endif // !DESCRIPTOR_SET_LAYOUT_H_
