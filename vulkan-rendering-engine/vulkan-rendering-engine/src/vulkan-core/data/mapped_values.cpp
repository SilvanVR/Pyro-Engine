#include "mapped_values.h"

#include "vulkan-core/memory_management/vulkan_memory_manager.h"
#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/vulkan_base.h"

#include <algorithm>
#include <assert.h>

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Static Member
    //---------------------------------------------------------------------------

    std::vector<MappedValues*> MappedValues::mappedValues;

    //---------------------------------------------------------------------------
    //  Init-Values for map-entries
    //---------------------------------------------------------------------------

    const int   defaultInt = 0;
    const float defaultFloat = 0.0f;
    const Vec2f defaultVec2f = Vec2f(0.0f, 0.0f);
    const Vec3f defaultVec3f = Vec3f(0.0f, 0.0f, 0.0f);
    const Vec4f defaultVec4f = Vec4f(0.0f, 0.0f, 0.0f, 0.0f);
    const Color defaultColor = Color::WHITE;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    MappedValues::MappedValues()
        : descriptorSetLayout(nullptr)
    {
        mappedValues.push_back(this);
    }

    MappedValues::MappedValues(const std::string& setName)
        : descriptorSetLayout(nullptr)
    {
        createDescriptorSets(setName);
        mappedValues.push_back(this);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    MappedValues::~MappedValues()
    {
        mappedValues.erase(std::remove(mappedValues.begin(), mappedValues.end(), this), mappedValues.end());
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Return the descriptor-set this shader is using
    //DescriptorSet* MappedValues::getDescriptorSet() 
    //{ 
    //    return descriptorSets[VulkanBase::getFrameDataIndex()].get();
    //}

    // bind this material to a command-buffer if it contains a descriptor-set
    void MappedValues::bind(VkCommandBuffer cmd)
    {
        assert(false);
    }

    //bind this descriptor-set to a command-buffer with the given pipeline-layout
    void MappedValues::bind(VkCommandBuffer cmd, PipelineLayout* pipelineLayout)
    {
        uint32_t frameDataIndex = VulkanBase::getFrameDataIndex();

        // Update current descriptor-set with the data from CPU (if the CPU-data is different)
        flush(frameDataIndex);

        // Finally bind the descriptor-set to the given CMD
        descriptorSets[frameDataIndex]->bind(cmd, pipelineLayout);
    }

    //---------------------------------------------------------------------------
    //  Public Methods - SET DATA
    //---------------------------------------------------------------------------

    // Change the used texture. If texture = nullptr, the default texture will be applied for that channel.
    void MappedValues::setTexture(const std::string& name, TexturePtr texture, const uint32_t& dstArrayElement)
    {
        // Update element in map only if new value is different
        // This prevents sending the same data's to the GPU again
        if (textureMap.count(name) == 0 || textureMap[name] != texture)
        {
            if (texture == nullptr)
                textureMap[name] = TEXTURE({ TEX_DEFAULT });
            else
                textureMap[name] = texture;
        }
        flushCounter = VulkanBase::numFrameDatas();
        lastFlushFrameDataIndex = -1;
    }

    // Set a color-value in this descriptor-set. A color is either a vec3 or a vec4
    void MappedValues::setColor(const std::string& name, const Color& color)
    {
        assert(vec3fMap.count(name) != 0 || vec4fMap.count(name) != 0);
        if (vec3fMap.count(name) != 0)
            vec3fMap[name] = color.getRGB();
        else
            vec4fMap[name] = color;

        flushCounter = VulkanBase::numFrameDatas();
        lastFlushFrameDataIndex = -1;
    }

    // Set a int-value in this descriptor-set.
    void MappedValues::setInt(const std::string& name, int val)
    {
        assert(intMap.count(name) != 0);
        intMap[name] = val;
        flushCounter = VulkanBase::numFrameDatas();
        lastFlushFrameDataIndex = -1;
    }

    // Set a float-value in this descriptor-set.
    void MappedValues::setFloat(const std::string& name, float val)
    {
        assert(floatMap.count(name) != 0);
        floatMap[name] = val;
        flushCounter = VulkanBase::numFrameDatas();
        lastFlushFrameDataIndex = -1;
    }

    // Set a vec2f in this descriptor-set.
    void MappedValues::setVec2f(const std::string& name, const Vec2f& vec)
    {
        assert(vec2fMap.count(name) != 0);
        vec2fMap[name] = vec;
        flushCounter = VulkanBase::numFrameDatas();
        lastFlushFrameDataIndex = -1;
    }

    // Set a vec3f in this descriptor-set.
    void MappedValues::setVec3f(const std::string& name, const Vec3f& vec)
    {
        assert(vec3fMap.count(name) != 0);
        vec3fMap[name] = vec;
        flushCounter = VulkanBase::numFrameDatas();
        lastFlushFrameDataIndex = -1;
    }

    // Set a vec4f in this descriptor-set.
    void MappedValues::setVec4f(const std::string& name, const Vec4f& vec)
    {
        assert(vec4fMap.count(name) != 0);
        vec4fMap[name] = vec;
        flushCounter = VulkanBase::numFrameDatas();
        lastFlushFrameDataIndex = -1;
    }

    // Set a mat4f in this descriptor-set.
    void MappedValues::setMat4f(const std::string& name, const Mat4f& mat)
    {
        assert(mat4fMap.count(name) != 0);
        mat4fMap[name] = mat;
        flushCounter = VulkanBase::numFrameDatas();
        lastFlushFrameDataIndex = -1;
    }


    //---------------------------------------------------------------------------
    //  Public Methods - SPECIAL USE CASES
    //---------------------------------------------------------------------------
    
    // Update a uniform-buffer with the given data. Use the whole uniform-buffer size if bufferSize = 0.
    //void MappedValues::setUniformBuffer(const std::string& name, void* data, const std::size_t& offset, const std::size_t& _bufferSize)
    //{
    //    const DescriptorLayoutBinding& binding = descriptorSetLayout->getBinding(name);

    //    std::size_t bufferSize = _bufferSize;
    //    if (bufferSize == 0) // use whole buffer-size
    //        bufferSize = static_cast<std::size_t>(binding.bufferSize);

    //    descriptorSets[VulkanBase::getFrameDataIndex()]->updateData(data, bufferSize, offset, binding.bindingNum);
    //}

    // Change the used texture with an image-info. Only for special use cases.
    void MappedValues::setTexture(const std::string& name, const VkDescriptorImageInfo& imageInfo, const uint32_t& dstArrayElement)
    {
        descriptorSets[VulkanBase::getFrameDataIndex()]->updateSet(&imageInfo, descriptorSetLayout->getBindingNum(name), dstArrayElement);
        flushCounter = VulkanBase::numFrameDatas();
    }

    // Change the used texture with an image-info. Only for special use cases.
    void MappedValues::setTexture(int index, const std::string& name, const VkDescriptorImageInfo& imageInfo)
    {
        descriptorSets[index]->updateSet(&imageInfo, descriptorSetLayout->getBindingNum(name));
        flushCounter = VulkanBase::numFrameDatas();
    }

    //---------------------------------------------------------------------------
    //  Public Methods - GET DATA
    //---------------------------------------------------------------------------

    // Return a reference of a used texture. Assert if not present.
    TexturePtr MappedValues::getTexture(const std::string& name)
    {
        assert(textureMap.count(name) != 0);
        return textureMap[name];
    }

    // Return a color for a given KEY. Assert if not present.
    Color MappedValues::getColor(const std::string& name)
    {
        assert(vec3fMap.count(name) != 0 || vec4fMap.count(name) != 0);
   
        if (vec3fMap.count(name) != 0)
        {
            Vec3f c = vec3fMap[name];
            return Color(c.r(), c.g(), c.b(), 1.0f);
        }
        else
        {
            Vec4f c = vec4fMap[name];
            return Color(c.r(), c.g(), c.b(), c.a());
        }
    }

    // Return a int value for a given KEY. Assert if not present.
    int MappedValues::getInt(const std::string& name)
    {
        assert(intMap.count(name) != 0);
        return intMap[name];
    }

    // Return a float value for a given KEY. Assert if not present.
    float MappedValues::getFloat(const std::string& name)
    {
        assert(floatMap.count(name) != 0);
        return floatMap[name];
    }

    // Return a vec2 value for a given KEY. Assert if not present.
    const Vec2f& MappedValues::getVec2f(const std::string& name)
    {
        assert(vec2fMap.count(name) != 0);
        return vec2fMap[name];
    }

    // Return a vec3 value for a given KEY. Assert if not present.
    const Vec3f& MappedValues::getVec3f(const std::string& name)
    {
        assert(vec3fMap.count(name) != 0);
        return vec3fMap[name];
    }

    // Return a vec4 value for a given KEY. Assert if not present.
    const Vec4f& MappedValues::getVec4f(const std::string& name)
    {
        assert(vec4fMap.count(name) != 0);
        return vec4fMap[name];
    }

    // Return a mat4f value for a given KEY. Assert if not present.
    const Mat4f& MappedValues::getMat4f(const std::string& name)
    {
        assert(mat4fMap.count(name) != 0);
        return mat4fMap[name];
    }

    DataType MappedValues::getDataType(const std::string& name)
    {
        return descriptorSetLayout != nullptr ? descriptorSetLayout->getDataType(name) : DataType::UNKNOWN;
    }

    //---------------------------------------------------------------------------
    //  Protected Methods
    //---------------------------------------------------------------------------

    // Creates the descriptor-set from a setName
    void MappedValues::createDescriptorSets(const std::string& setName)
    {
        assert(!hasDescriptorSets);

        descriptorSetLayout = VMM::getSetLayout(setName);

        if (descriptorSetLayout == nullptr)
            Logger::Log("MappedValues::createDescriptorSet(): Given Descriptor-Set-Name '"+ setName + "' does not exist.", LOGTYPE_ERROR);

        // Call function below
        createDescriptorSets(descriptorSetLayout);
    }

    // Creates the descriptor-set from a setLayout
    void MappedValues::createDescriptorSets(DescriptorSetLayout* setLayout)
    {
        assert(!hasDescriptorSets);

        descriptorSetLayout = setLayout;
        for (int i = 0; i < VulkanBase::numFrameDatas(); i++)
        {
            std::unique_ptr<DescriptorSet> ds(VMM::createDescriptorSet(descriptorSetLayout));
            descriptorSets.push_back(std::move(ds));
        }

        hasDescriptorSets = true;
        flushCounter = VulkanBase::numFrameDatas();

        initMapDatas();
    }

    // Delete old descriptor-sets and create new ones from the given setLayout
    void MappedValues::changeDescriptorSets(const std::string& setName)
    {
        reset();
        createDescriptorSets(setName);
    }
    
    // Delete old descriptor-sets and create new ones from the given setLayout
    void MappedValues::changeDescriptorSets(DescriptorSetLayout* setLayout)
    {
        reset();
        if (setLayout != nullptr)
            createDescriptorSets(setLayout);
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Initialize maps with name from the descriptor-set-layout
    void MappedValues::initMapDatas()
    {
        // Fill maps with "empty-datas"
        for (const auto& binding : descriptorSetLayout->getBindings())
        {
            for (const auto& member : binding.bufferRanges)
            {
                const std::string& name = member.name;

                // Detect type and fill apropriate map
                switch (member.dataType)
                {
                case DataType::Mat4:   mat4fMap[name] = Mat4f::identity;    break;
                case DataType::Int:    intMap[name]   = defaultInt;         break;
                case DataType::Float:  floatMap[name] = defaultFloat;       break;
                case DataType::Vec2:   vec2fMap[name] = defaultVec2f;       break;
                case DataType::Vec3:   vec3fMap[name] = defaultVec3f;       break;
                case DataType::Vec4:   vec4fMap[name] = defaultVec4f;       break;
                default:
                    Logger::Log("UNSUPPORTED DATA-TYPE in MappedValues::initMapDatas(): " + (int)member.dataType, LOGTYPE_WARNING);
                }
            }
        }
    }

    // Small helper functions for the set.. - functions
    void MappedValues::updateBufferData(const std::string& name, void* data, uint32_t frameDataIndex)
    {
        const BufferRange& bufferRange = descriptorSetLayout->getBufferRange(name);
        descriptorSets[frameDataIndex]->updateData(data, bufferRange.range, bufferRange.offset, descriptorSetLayout->getBindingNum(name));
    }

    // Send data's stored in the RAM to the GPU if necessary
    void MappedValues::flush(uint32_t frameDataIndex)
    {
        // Protect the flushing, so it can occur only if some datas has changed
        if(flushCounter == 0 || lastFlushFrameDataIndex == frameDataIndex)
            return;

        lastFlushFrameDataIndex = frameDataIndex;

        // Decrement flushCounter. If its zero all descriptor-sets are up to date
        if (flushCounter >= 1) flushCounter--;

        // Loop through all entries in the texture-map and update the descriptor-set
        for (auto& e : textureMap)
        {
            TexturePtr bla = e.second;
            VulkanTextureResource* res = bla->getVulkanTextureResource();
            descriptorSets[frameDataIndex]->updateSet(e.second->getVulkanTextureResource()->getDescriptorImageInfo(),
                                                      descriptorSetLayout->getBindingNum(e.first));
        }

        // Loop through all entries in all maps and send the data to the GPU.
        for (auto& e : intMap)
            updateBufferData(e.first, &e.second, frameDataIndex);

        for (auto& e : floatMap)
            updateBufferData(e.first, &e.second, frameDataIndex);

        for (auto& e : vec2fMap)
            updateBufferData(e.first, &e.second, frameDataIndex);

        for (auto& e : vec3fMap)
            updateBufferData(e.first, &e.second, frameDataIndex);

        for (auto& e : vec4fMap)
            updateBufferData(e.first, &e.second, frameDataIndex);

        for (auto& e : mat4fMap)
            updateBufferData(e.first, &e.second, frameDataIndex);
    }

    // Destroy all descriptor-sets and clean-up maps
    // Useful for classes which want to change their descriptor-set
    void MappedValues::reset()
    {
        descriptorSetLayout = nullptr;
        hasDescriptorSets = false;
        lastFlushFrameDataIndex = -1;
        descriptorSets.clear();
        textureMap.clear();
        intMap.clear();
        floatMap.clear();
        vec2fMap.clear();
        vec3fMap.clear();
        vec4fMap.clear();
        mat4fMap.clear();
    }

    // Print all names in all Maps with their data-types (For DEBUGGING)
    void MappedValues::printAllNames()
    {
        std::string finalText = " << Descriptor-Set-Layout: " + descriptorSetLayout->getName() + " >> \n";

        for (auto& e : intMap)
            finalText += "[Int] " + e.first + "\n";

        for (auto& e : floatMap)
            finalText += "[Float] " + e.first + "\n";

        for (auto& e : vec2fMap)
            finalText += "[Vec2f] " + e.first + "\n";

        for (auto& e : vec3fMap)
            finalText += "[Vec3f] " + e.first + "\n";

        for (auto& e : vec4fMap)
            finalText += "[Vec4f] " + e.first + "\n";

        for (auto& e : mat4fMap)
            finalText += "[Ma4f] " + e.first + "\n";

        std::cout << finalText << std::endl;
    }

    // Print all names with values in all Maps with their data-types (For DEBUGGING)
    void MappedValues::printAllNamesWithValues()
    {
        std::cout << " << Descriptor-Set-Layout: " << descriptorSetLayout->getName() << " >>" << std::endl;

        for(auto& e : textureMap)
            std::cout << "[Texture] " << e.first << ": " << e.second->getName() << std::endl;

        for (auto& e : intMap)
            std::cout << "[Int] " << e.first << ": " << e.second << std::endl;

        for (auto& e : floatMap)
            std::cout << "[Float] " << e.first << ": " << e.second << std::endl;

        for (auto& e : vec2fMap)
            std::cout << "[Vec2f] " << e.first << ": " << e.second << std::endl;

        for (auto& e : vec3fMap)
            std::cout << "[Vec3f] " << e.first << ": " << e.second << std::endl;

        for (auto& e : vec4fMap)
            std::cout << "[Vec4f] " << e.first << ": " << e.second << std::endl;

        for (auto& e : mat4fMap)
            std::cout << "[Mat4f] " << e.first << ": " << e.second << std::endl;
    }





}