#include "shader_module.h"

#include "spirv_cross/spirv_glsl.hpp"       //SPIR-V reflection with spirv-cross
#include "file_system/vfs.h"

namespace Pyro
{

    // Forward declaration. See function for explanation.
    std::vector<BufferRange> parseUniformStruct(const spirv_cross::Compiler& comp, const spirv_cross::SPIRType& spirType,
                                                uint32_t parentOffset, const std::string& parentName);

    // Return the appropriate vertex-layout for a vertex-shader.
    VertexLayout parseVertexLayout(const std::string& filePath, const spirv_cross::Compiler& comp, const spirv_cross::ShaderResources& resources);

    // Return a data-type for a buffer-member.
    DataType getDataType(const spirv_cross::SPIRType& memberType);

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    ShaderModule::ShaderModule(VkDevice _device, const std::string& path, const ShaderStage& _shaderStage)
        : device(_device), shaderStage(_shaderStage), filePath(path)
    {
        // Read SPIR-V and create a VkShaderModule
        std::vector<uint32_t> spv = FileSystem::readBinaryFile(filePath.c_str());

        // Create Descriptor-Set-Layouts from the used descriptor-sets in the shader
        parseDescriptorSets(spv);

        VkShaderModuleCreateInfo vertShaderCreateInfo = {};
        vertShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertShaderCreateInfo.pNext = nullptr;
        vertShaderCreateInfo.flags = 0;
        vertShaderCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
        vertShaderCreateInfo.pCode = spv.data();

        VkResult res = vkCreateShaderModule(device, &vertShaderCreateInfo, nullptr, &shader);
        assert(res == VK_SUCCESS);
    }


    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    // Destroy the VkShaderModule and set layouts
    ShaderModule::~ShaderModule()
    {
        vkDestroyShaderModule(device, shader, nullptr);
        for (auto& setLayout : descriptorSetLayouts)
            delete setLayout;
    };


    //---------------------------------------------------------------------------
    //  Private Functions
    //---------------------------------------------------------------------------

    enum class DescriptorSetOption
    {
        DYNAMIC_UNIFORM_BUFFER = 'D',
        SHADER_SET = 'S',
        MATERIAL_SET = 'M'
    };

    enum class DSOption
    {
        DYNAMIC_UNIFORM_BUFFER = 1,
        SHADER_SET = 2,
        MATERIAL_SET = 4
    };

    // Get the option-val for a binding (e.g. Material_D -> dynamic-uniform-buffer) and erase those 
    unsigned int getOption(const std::string& name)
    {
        unsigned int optionBitField = 0;

        for (unsigned int i = 0; i < name.size(); i++)
        {
            if (name[i] == '_')
            {
                for (unsigned int j = i + 1; j < name.size(); j++)
                {
                    switch (name[j])
                    {
                    case (char)DescriptorSetOption::DYNAMIC_UNIFORM_BUFFER:
                        optionBitField |= (unsigned int)DSOption::DYNAMIC_UNIFORM_BUFFER;
                        break;
                    case (char)DescriptorSetOption::SHADER_SET:
                        optionBitField |= (unsigned int)DSOption::SHADER_SET;
                        break;
                    case (char)DescriptorSetOption::MATERIAL_SET:
                        optionBitField |= (unsigned int)DSOption::MATERIAL_SET;
                        break;
                    }
                }
                break;
            }
        }

        return optionBitField;
    }

    void removeOptionCharacters(std::string& name)
    {
        int optionIndex = -1;

        for (unsigned int i = 0; i < name.size(); i++)
        {
            if (name[i] == '_')
            {
                optionIndex = static_cast<int>(name.size()) - i;
                break;
            }
        }
        if (optionIndex != -1)
            name.erase(name.end() - optionIndex, name.end());  // Cut-Off last two characters
    }

    // Parse the SPIR-V shader-text using spirv-cross and create appropriate descriptor-set-layouts
    void ShaderModule::parseDescriptorSets(const std::vector<uint32_t>& spv)
    {
        // Parse spirv
        spirv_cross::Compiler comp(spv);

        // The SPIR-V is now parsed, and we can perform reflection on it.
        spirv_cross::ShaderResources resources = comp.get_shader_resources();

        VkShaderStageFlags vkShaderStage = -1;
        switch (shaderStage)
        {
        case ShaderStage::Vertex:
            vkShaderStage = VK_SHADER_STAGE_VERTEX_BIT;
            vertexLayout = parseVertexLayout(filePath, comp, resources);
            break;
        case ShaderStage::Fragment:
            vkShaderStage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
        case ShaderStage::Geometry:
            vkShaderStage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
        case ShaderStage::Tessellation:
            vkShaderStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
        }
        assert(vkShaderStage != -1);

        // First int: Set-Number, Second int: Binding-Numer. Generally this sorts the bindings by set- / binding- number.
        std::map<int, std::map<int, DescriptorLayoutBinding>> sets;

        // Uniforms
        for (auto &resource : resources.uniform_buffers)
        {
            uint32_t setNum = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            uint32_t bindingNum = comp.get_decoration(resource.id, spv::DecorationBinding);

            // Get buffer-size
            const spirv_cross::SPIRType& type = comp.get_type(resource.type_id);

            // Get Array-Size
            uint32_t arraySize = 1;
            if (type.array.size() != 0)
                arraySize = type.array[0];

            // Get Buffer-Size
            std::size_t bufferSize = comp.get_declared_struct_size(type);

            // Check if the UBO should be dynamic ( "_D" at the end of the name)
            VkDescriptorType bufferType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            // Change descriptor-type if it should a dynamic uniform-buffer
            if (getOption(resource.name) & (unsigned int)DSOption::DYNAMIC_UNIFORM_BUFFER)
                bufferType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

            // Get name, offsets and ranges for individual data-members in a struct. Recursive algorithm which reads also structs in structs and so on.
            std::vector<BufferRange> memberRanges = parseUniformStruct(comp, type, 0, "");

            // Bindings are automatically sorted in the second <map>
            sets[setNum][bindingNum] = { bufferType, arraySize, vkShaderStage, resource.name, 
                                         bufferSize, memberRanges, bindingNum, DataType::Struct };
        }

        // Multiple Uniform-Buffers
        for (auto &resource : resources.storage_buffers)
        {
            assert(0 && "UNTESTED CODE. MIGHT NOT WORK");
            unsigned int setNum = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned int bindingNum = comp.get_decoration(resource.id, spv::DecorationBinding);

            const spirv_cross::SPIRType& type = comp.get_type(resource.type_id);

            // Get Array-Size
            uint32_t arraySize = 1;
            if (type.array.size() != 0)
                arraySize = type.array[0];

            // Get Buffer-Size
            std::size_t bufferSize = comp.get_declared_struct_size(type);

            // Get name, offsets and ranges for individual data-members in a struct. Recursive algorithm which reads also structs in structs and so on.
            std::vector<BufferRange> memberRanges = parseUniformStruct(comp, type, 0, "");

            // Bindings are automatically sorted in the second <map>
            sets[setNum][bindingNum] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, arraySize, vkShaderStage, 
                                         resource.name, bufferSize, memberRanges, bindingNum, DataType::Struct };
        }

        // Image Sampler
        for (auto &resource : resources.sampled_images)
        {
            unsigned int setNum = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned int bindingNum = comp.get_decoration(resource.id, spv::DecorationBinding);

            const spirv_cross::SPIRType& type = comp.get_type(resource.type_id);

            // Get Array-Size
            uint32_t arraySize = 1;
            if(type.array.size() != 0)
                arraySize = type.array[0];
            
            // Detect data-type
            DataType dataType = DataType::Texture2D;
            if(type.image.dim == spv::Dim::DimCube)
                dataType = DataType::TextureCubemap;

            // Bindings are automatically sorted in the second <map>
            sets[setNum][bindingNum] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, arraySize, vkShaderStage, 
                                         resource.name, 0, {}, bindingNum, dataType };
        }

        // Multiple Image Sampler
        for (auto &resource : resources.storage_images)
        {
            assert(0 && "UNTESTED CODE. MIGHT NOT WORK");
            unsigned int setNum = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned int bindingNum = comp.get_decoration(resource.id, spv::DecorationBinding);

            const spirv_cross::SPIRType& type = comp.get_type(resource.type_id);

            // Get Array-Size
            uint32_t arraySize = 1;
            if (type.array.size() != 0)
                arraySize = type.array[0];

            // Bindings are automatically sorted in the second <map>
            sets[setNum][bindingNum] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, arraySize, vkShaderStage, 
                                         resource.name, 0, {}, bindingNum, DataType::Texture2D };
        }

        // Push-Constants
        for (auto &resource : resources.push_constant_buffers)
        {
            const spirv_cross::SPIRType& type = comp.get_type(resource.type_id);

            VkPushConstantRange pushConstantRange;
            pushConstantRange.stageFlags = vkShaderStage;
            pushConstantRange.offset = 0; // TODO: GET OFFSET
            pushConstantRange.size = static_cast<uint32_t>(comp.get_declared_struct_size(type));

            pushConstants.push_back({ pushConstantRange, resource.name });
        }

        // For each Descriptor-Set create a layout.
        for (auto& iterator = sets.begin(); iterator != sets.end(); iterator++)
        {
            int         setNumber = iterator->first;
            std::string setName = "";

            // Will contain all Bindings from one Descriptor-Set in a sorted order
            std::vector<DescriptorLayoutBinding> shaderBindings;

            bool isMaterialSet = false;
            bool isShaderSet = false;

            // Iterate over all bindings in the descriptor-set
            for (auto& binding : iterator->second)
            {
                // Check the option-fields for every binding
                unsigned int option = getOption(binding.second.name);

                if (option & (unsigned int)DSOption::MATERIAL_SET)
                    isMaterialSet = true;
                if (option & (unsigned int)DSOption::SHADER_SET)
                    isShaderSet = true;

                // Remove the option characters
                removeOptionCharacters(binding.second.name);

                // Take name from a binding which first letter is in upper-case as the SET-NAME
                if (isupper(binding.second.name[0]) && setName == "")
                    setName = binding.second.name;

                shaderBindings.push_back(binding.second);
            }

            assert(setName != ""); // No Setname found 
            assert(!(isMaterialSet && isShaderSet)); //both MaterialSet and ShaderSet are specified, which is not allowed

            // Create the layout and save it in this ShaderModule-Class
            DescriptorSetLayout* newSetLayout = new DescriptorSetLayout(device, setName, setNumber, shaderBindings);

            if (isShaderSet)
                newSetLayout->m_isShaderSet = true;
            if (isMaterialSet)
                newSetLayout->m_isMaterialSet = true;

            descriptorSetLayouts.push_back(newSetLayout);
        }

    }

    // Parses the vertex-layout and return it in form of a class
    VertexLayout parseVertexLayout(const std::string& filePath, const spirv_cross::Compiler& comp, const spirv_cross::ShaderResources& resources)
    {
        // Sorted Layouts by locations
        std::map<uint32_t, VertexLayout::Layout> layoutMap;

        for (const auto& stageInput : resources.stage_inputs)
        {
            const spirv_cross::SPIRType& type = comp.get_type(stageInput.type_id);
            uint32_t location = comp.get_decoration(stageInput.id, spv::Decoration::DecorationLocation);

            switch (type.vecsize)
            {
            case 2:
                layoutMap[location] = VertexLayout::Layout::VEC2F;
                break;
            case 3:
                layoutMap[location] = VertexLayout::Layout::VEC3F;
                break;
            default:
                Logger::Log("ShaderModule::parseVertexLayout(): Given Variable-Type from '" + stageInput.name + "' is not supported yet "
                            "in shader: '" + filePath + "'. Go and ADD IT in shader_module.cpp!!!", LOGTYPE_ERROR);
            }
        }

        // Make the SORTED <map> to a <vector>
        std::vector<VertexLayout::Layout> layouts;

        for (const auto& layout : layoutMap)
            layouts.push_back(layout.second);

        return VertexLayout(layouts);
    }

    // Parse a UBO and return a list of names, offsets and size of the individual variables
    std::vector<BufferRange> parseUniformStruct(const spirv_cross::Compiler& comp, const spirv_cross::SPIRType& spirType,
                                                uint32_t parentOffset, const std::string& parentName)
    {
        std::vector<BufferRange> memberRanges;

        for (unsigned int i = 0; i < spirType.member_types.size(); i++)
        {
            // Get the member-name and add the "parent"-name infront of it e.g. "baseLight-" + color
            const std::string& memberName = parentName + comp.get_member_name(spirType.self, i);
            // Get the offset of the member and add the parent-offset to it
            uint32_t offset = parentOffset + comp.get_member_decoration(spirType.self, i, spv::DecorationOffset);

            const spirv_cross::SPIRType& memberType = comp.get_type(spirType.member_types[i]);

            if (memberType.basetype == spirv_cross::SPIRType::Struct)
            {
                // Member is itself a struct, so call this function with that member and add all his members to the vector "memberRanges"
                std::vector<BufferRange> br = parseUniformStruct(comp, memberType, offset, memberName + ".");
                memberRanges.insert(std::end(memberRanges), std::begin(br), std::end(br));
            }
            else
            {
                // Member is not a struct. Get the size and type of it and push it to the vector.
                uint32_t range = static_cast<uint32_t>(comp.get_declared_struct_member_size(spirType, i));
                DataType dataType = getDataType(memberType);
                memberRanges.push_back({ memberName, offset, range, i, dataType });
            }

        }

        return memberRanges;
    }


    // Return a data-type for a buffer-member.
    DataType getDataType(const spirv_cross::SPIRType& memberType)
    {
        DataType dataType = DataType::UNKNOWN;

        // vecsize equal to ROW
        switch (memberType.vecsize)
        {
            // Not a vector nor a matrix with row == 1
            case 1:
                switch (memberType.basetype)
                {
                case spirv_cross::SPIRType::Float:
                    dataType = DataType::Float; break;
                case spirv_cross::SPIRType::Double:
                    dataType = DataType::Double; break;
                case spirv_cross::SPIRType::Int:
                    dataType = DataType::Int; break;
                case spirv_cross::SPIRType::Boolean:
                    dataType = DataType::Boolean; break;
                case spirv_cross::SPIRType::Char:
                    dataType = DataType::Char; break;
                }
                break;
            case 2:
                if(memberType.columns == 1)
                    dataType = DataType::Vec2; break;
            case 3:
                if(memberType.columns == 1)
                    dataType = DataType::Vec3; break;
            case 4:
                if (memberType.columns == 1)
                    dataType = DataType::Vec4;
                else if(memberType.columns == 4)
                    dataType = DataType::Mat4;
                break;
        }

        if(dataType == DataType::UNKNOWN)
            Logger::Log("Unrecognized Data-Typed parsed in ShaderModule::getDataType()", LOGTYPE_WARNING);

        return dataType;
    }

}