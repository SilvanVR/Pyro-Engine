#include "descriptor_set.h"

#include "../pipeline_layout/pipeline_layout.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "vulkan-core/vulkan_base.h"   // access to the system-struct
#include "descriptor_set_layout.h"


#include <assert.h>

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    DescriptorSet::DynamicDescriptorSetManager DescriptorSet::dynamicSetManager;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    // Create a DescriptorSet from a given layout. The name from the layout will be used to find the slot in a pipeline-layout
    DescriptorSet::DescriptorSet(VkDevice _device, DescriptorSetLayout* layout)
        : device(_device), setLayout(layout)
    {
        // Uniform-Buffer will be created later in the DescriptorPoolManager, because we have to allocate space for the VkDescriptorSet first
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    DescriptorSet::~DescriptorSet()
    {
        for (auto& buf : bufferBindings)
            delete buf.second;
    }

    //---------------------------------------------------------------------------
    //  Public Methods - Update Set
    //---------------------------------------------------------------------------

    // Update the descriptor set with a VkDescriptorBufferInfo
    void DescriptorSet::updateSet(const VkDescriptorBufferInfo* bufferInfo, const uint32_t& dstBinding, const uint32_t& dstArrayElement)
    {
        if (descriptorSet == VK_NULL_HANDLE)
            Logger::Log("Called update() on a descriptor set which VkDescriptorSet = VK_NULL_HANDLE", LOGTYPE_ERROR);

        VkWriteDescriptorSet writes[1];
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].pNext = nullptr;
        writes[0].dstSet = descriptorSet;
        writes[0].dstBinding = dstBinding;
        writes[0].dstArrayElement = dstArrayElement;
        writes[0].descriptorCount = 1;
        writes[0].descriptorType = setLayout->getBinding(dstBinding).type;
        writes[0].pBufferInfo = bufferInfo;

        vkUpdateDescriptorSets(device, 1, writes, 0, NULL);
    }


    // Update the descriptor set with a VkDescriptorImageInfo. descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    void DescriptorSet::updateSet(const VkDescriptorImageInfo* imageInfo, const uint32_t& dstBinding, const uint32_t& dstArrayElement)
    {
        if (descriptorSet == VK_NULL_HANDLE)
            Logger::Log("Called update() on a descriptor set which VkDescriptorSet = VK_NULL_HANDLE", LOGTYPE_ERROR);

        VkWriteDescriptorSet writes[1];
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].pNext = nullptr;
        writes[0].dstSet = descriptorSet;
        writes[0].dstBinding = dstBinding;
        writes[0].dstArrayElement = dstArrayElement;
        writes[0].descriptorCount = 1;
        writes[0].descriptorType = setLayout->getBinding(dstBinding).type;
        writes[0].pImageInfo = imageInfo;

        vkUpdateDescriptorSets(device, 1, writes, 0, NULL);
    }


    // Update the descriptor set with unlimited image-infos
    void DescriptorSet::updateSet(const std::vector<ImageWriteDescriptorSet>& imageWrites)
    {
        if (descriptorSet == VK_NULL_HANDLE)
            Logger::Log("Called update() on a descriptor set which VkDescriptorSet = VK_NULL_HANDLE", LOGTYPE_ERROR);

        std::vector<VkWriteDescriptorSet> writes;

        for (auto& imageWrite : imageWrites)
        {
            VkWriteDescriptorSet newWrite;
            newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            newWrite.pNext = nullptr;
            newWrite.dstSet = descriptorSet;
            newWrite.dstBinding = imageWrite.dstBinding;
            newWrite.dstArrayElement = 0;
            newWrite.descriptorCount = 1;
            newWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            newWrite.pImageInfo = imageWrite.imageInfo;

            writes.push_back(newWrite);
        }

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(imageWrites.size()), writes.data(), 0, NULL);
    }


    //---------------------------------------------------------------------------
    //  Public Methods - Update Data
    //---------------------------------------------------------------------------

    // Update the uniform buffer with the given data. It transfers bufferSize to the gpu. If bufferSize = 0, it take the whole size.
    void DescriptorSet::updateData(void* data, const VkDeviceSize& bufferSize, const std::size_t& offset, const uint32_t& dstBinding)
    {
        assert(setLayout->getBinding(dstBinding).type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
               setLayout->getBinding(dstBinding).type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
               && "ERROR in DescriptorSet::updateData(). Given dstBinding is not a uniform-buffer.");

        // Update the uniform-buffer behind that binding (normal or a dynamic)
        bufferBindings[dstBinding]->updateData(data, bufferSize, offset, dstBinding);
    }

    // Update the uniform buffer with the given data. It transfers bufferSize to the gpu. If bufferSize = 0, it take the whole size.
    void DescriptorSet::UniformBufferSuper::updateData(void* data, const VkDeviceSize& bufferSize, const std::size_t& offset, const uint32_t& dstBinding)
    {
        uint8_t *pDataOffset = pData + offset;
        if (bufferSize == 0) // Take whole buffer-size if the variable is zero
        {
            // Send data to the GPU
            memcpy(pDataOffset, data, static_cast<std::size_t>(bufferInfo.range));
        }
        else
        {
            // Check that the given buffer size does not exceed the total buffer size
            assert(bufferInfo.range >= (offset + bufferSize) && "ERORR in DescriptorSet::updateData(): Buffer-Overflow!");

            // Send data to the GPU
            memcpy(pDataOffset, data, static_cast<std::size_t>(bufferSize));
        }
    }

    //---------------------------------------------------------------------------
    //  Public Methods - Bind
    //---------------------------------------------------------------------------

    // Bind the descriptor set to the given Command Buffer. Automatically find the appropriate set number in the given pipeline layout if present.
    void DescriptorSet::bind(VkCommandBuffer cmd, PipelineLayout* pipelineLayout)
    {
        uint32_t setNumber = pipelineLayout->getSetNumber(setLayout);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->get(), setNumber,
                                1, &descriptorSet, static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
    }

    // Bind a bunch of descriptor-sets together. The first-set in "sets" is the beginning set-number. 
    // Second set must have set-number "previous-set + 1" etc.
    void DescriptorSet::bind(VkCommandBuffer cmd, const std::vector<DescriptorSet*>& sets, PipelineLayout* pipelineLayout)
    {
        std::vector<VkDescriptorSet> vkSets;
        std::vector<uint32_t> dynamicOffsets;

        for (const auto& set : sets)
        {
            // Add VkDescriptorSet
            vkSets.push_back(set->get());
            // Add all dynamic-offsets
            dynamicOffsets.insert(dynamicOffsets.end(), set->dynamicOffsets.begin(), set->dynamicOffsets.end());
        }

        // Use set-layout from the first set to find the set-number in the pipeline-layout
        DescriptorSetLayout* setLayout = sets[0]->getSetLayout();

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->get(), pipelineLayout->getSetNumber(setLayout),
                                static_cast<uint32_t>(vkSets.size()), vkSets.data(), static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
    }

    //---------------------------------------------------------------------------
    //  Private Friend Methods
    //---------------------------------------------------------------------------

    // Create a VkBuffer (and allocate memory) and update the descriptor-set for every Uniform-Buffer-Binding in the set-layout.
    void DescriptorSet::createUniformBuffers()
    {
        // Get all bindings from that layout and create an appropriate buffer for it if needed
        const std::vector<DescriptorLayoutBinding>& bindings = setLayout->getBindings();

        for (unsigned int i = 0; i < bindings.size(); i++)
        {
            if (bindings[i].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                createBuffer(i, bindings[i].bufferSize);
            else if (bindings[i].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
            {
                // Algorithm:
                // 1.) Check if a dynamic-uniform-buffer exists for a set-layout and a given binding AND IF ENOUGH SPACE
                // 2.) NO:  Allocate a dynamic buffer.
                // 3.) YES: Update binding with the bufferInfo from the one who created the buffer. Get Dynamic-Offset.

                // Check if an buffer for that dynamic-uniform has already been created. If not, do so.
                if (!dynamicSetManager.hasBuffer(setLayout, i))
                {
                    // No buffer found, create one
                    createDynamicUniformBuffer(i, bindings[i].bufferSize);
                }
                else
                {
                    // Check if the last created buffer has still room for a new set.
                    if (!dynamicSetManager.enoughSpace(setLayout, i))
                    {
                        // Wasnt enough space, so create another one
                        createDynamicUniformBuffer(i, bindings[i].bufferSize);
                    }
                    else
                    {
                        // Enough space is available, so get everything needed from it.
                        DynamicUniformBuffer* dynamicBuffer = new DynamicUniformBuffer;

                        dynamicBuffer->bufferInfo = dynamicSetManager.getBufferInfo(setLayout, i);
                        dynamicBuffer->dynamicOffset = dynamicSetManager.getDynamicOffset(setLayout, i);

                        // Directly calculate the data-pointer pointing to the block of GPU memory for this dynamic-uniform-buffer
                        dynamicBuffer->pData = dynamicSetManager.getDataPtr(setLayout, i) + dynamicBuffer->dynamicOffset;

                        // Update set with the new information
                        updateSet(&dynamicBuffer->bufferInfo, i);

                        // Add the dynamic-offset to the list.
                        dynamicOffsets.push_back(dynamicBuffer->dynamicOffset);

                        // Add it to the map.
                        bufferBindings[i] = dynamicBuffer;
                    }

                }
            }
        }
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Basically creates an normal buffer, large enough for all sets and add some information to the dynamicSetManager
    void DescriptorSet::createDynamicUniformBuffer(uint32_t dstBinding, VkDeviceSize bufferSize)
    {
        // Create a buffer, large enough holding "maxChildsDynamicSet"
        createBuffer(dstBinding, bufferSize, maxChildsDynamicSet);

        // Add necessary information to the set-manager
        dynamicSetManager.add(setLayout, dstBinding, bufferBindings[dstBinding]);

        // Add a "zero"-offset for this set aswell
        dynamicOffsets.push_back(0);
    }

    // Create an normal host-visible buffer for a binding and updates the set
    void DescriptorSet::createBuffer(uint32_t dstBinding, VkDeviceSize bufferSize, uint32_t objectCount)
    {
        UniformBuffer* uniform = new UniformBuffer;

        if (VulkanBase::getGPU().properties.limits.minUniformBufferOffsetAlignment)
            bufferSize = (bufferSize +
                VulkanBase::getGPU().properties.limits.minUniformBufferOffsetAlignment - 1) &
            ~(VulkanBase::getGPU().properties.limits.minUniformBufferOffsetAlignment - 1);

        // Create a vulkan buffer, allocate device memory and bind it
        uniform->buffer = std::unique_ptr<VulkanBuffer>(new VulkanBuffer(device, bufferSize * objectCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

        // Store information in the uniform's descriptor
        uniform->bufferInfo = uniform->buffer->getDescriptorBufferInfo(0, bufferSize);

        updateSet(&uniform->bufferInfo, dstBinding);

        // Map Memory Once (Unmapped automatically when buffer gets destroyed)
        uniform->pData = (uint8_t*)uniform->buffer->map(0, uniform->bufferInfo.range);

        bufferBindings[dstBinding] = uniform;
    }

    //---------------------------------------------------------------------------
    //  DynamicUniformData-Class
    //---------------------------------------------------------------------------

    // Calculate and return the dynamic offset. Exception if not enough space.
    uint32_t DescriptorSet::DynamicDescriptorSetManager::DynamicUniformData::getDynamicOffset()
    {
        if (numDynamicSets + 1 >= maxChildsDynamicSet)
            Logger::Log("DynamicDescriptorSet::getDynamicOffset(): Too many objects!", LOGTYPE_ERROR);

        numDynamicSets++;
        uint32_t dynamicOffset = numDynamicSets * static_cast<uint32_t>(bufferInfo.range);

        return dynamicOffset;
    }

    //---------------------------------------------------------------------------
    //  DynamicDescriptorSetManager-Class
    //---------------------------------------------------------------------------

    // Check if an buffer already exists for a set-layout and binding-num
    bool DescriptorSet::DynamicDescriptorSetManager::hasBuffer(DescriptorSetLayout* setLayout, uint32_t dstBinding)
    {
        return dynamicUniformBuffers[setLayout].count(dstBinding) != 0;
    }

    // Check if still enough place in the dynamic buffer
    bool DescriptorSet::DynamicDescriptorSetManager::enoughSpace(DescriptorSetLayout* setLayout, uint32_t dstBinding, uint32_t setCount)
    {
        return (dynamicUniformBuffers[setLayout][dstBinding].back().numDynamicSets + setCount) < maxChildsDynamicSet;
    }

    // Return the buffer-info object for the last created dynamic-buffer
    VkDescriptorBufferInfo& DescriptorSet::DynamicDescriptorSetManager::getBufferInfo(DescriptorSetLayout* setLayout, uint32_t dstBinding)
    {
        return dynamicUniformBuffers[setLayout][dstBinding].back().bufferInfo;
    }

    // Calculate and return a dynamic offset
    uint32_t DescriptorSet::DynamicDescriptorSetManager::getDynamicOffset(DescriptorSetLayout* setLayout, uint32_t dstBinding)
    {
        return dynamicUniformBuffers[setLayout][dstBinding].back().getDynamicOffset();
    }

    // Add the (dynamic) uniform-buffer to this manager
    void DescriptorSet::DynamicDescriptorSetManager::add(DescriptorSetLayout* setLayout, uint32_t dstBinding, const UniformBufferSuper* uniformBuffer)
    {
        dynamicUniformBuffers[setLayout][dstBinding].push_back({ uniformBuffer->bufferInfo, 0, uniformBuffer->pData });
    }

    uint8_t* DescriptorSet::DynamicDescriptorSetManager::getDataPtr(DescriptorSetLayout* setLayout, uint32_t dstBinding)
    {
        return dynamicUniformBuffers[setLayout][dstBinding].back().dataPtr;
    }

}