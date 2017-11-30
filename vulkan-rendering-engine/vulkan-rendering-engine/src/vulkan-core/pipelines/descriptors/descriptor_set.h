#ifndef DESCRIPTOR_SET_H_
#define DESCRIPTOR_SET_H_

#include "build_options.h"
#include "vulkan-core/util_classes/vulkan_buffer.h"

namespace Pyro
{


    class DescriptorPool;
    class PipelineLayout;
    class DescriptorSetLayout;

    const uint32_t maxChildsDynamicSet = 1024;

    //---------------------------------------------------------------------------
    //  DescriptorSet Class
    //---------------------------------------------------------------------------

    class DescriptorSet
    {
        friend class DescriptorPoolManager; // Allow the manager to access createUniformBuffers()

        // Update the descriptor set with a VkDescriptorBufferInfo
        void updateSet(const VkDescriptorBufferInfo* bufferInfo, const uint32_t& dstBinding = 0, const uint32_t& dstArrayElement = 0);

    public:
        struct ImageWriteDescriptorSet
        {
            VkDescriptorImageInfo*  imageInfo;
            uint32_t                dstBinding;
        };

        // Create a DescriptorSet from a given layout. The name from the layout will be used to find the slot in a pipeline-layout
        DescriptorSet(VkDevice device, DescriptorSetLayout* layout);
        ~DescriptorSet();

        // Return the VkDescriptorSet
        VkDescriptorSet& get() { return descriptorSet; }

        // Return the descriptor-set-layout this descriptor-set is using
        DescriptorSetLayout* getSetLayout() { return setLayout; }

        //---------------------------------------------------------------------------
        //  Update Set
        //---------------------------------------------------------------------------

        // Update the descriptor set with a VkDescriptorImageInfo
        void updateSet(const VkDescriptorImageInfo* imageInfo, const uint32_t& dstBinding = 0, const uint32_t& dstArrayElement = 0);

        // Update the descriptor set with unlimited binding-infos
        void updateSet(const std::vector<ImageWriteDescriptorSet>& imageWrites);

        //---------------------------------------------------------------------------
        //  Update Data
        //---------------------------------------------------------------------------

        // Update the uniform buffer with the given data. If bufferSize = 0, it will take the whole size of the buffer.
        void updateData(void* data, const VkDeviceSize& bufferSize = 0, const std::size_t& offset = 0, const uint32_t& dstBinding = 0);

        //---------------------------------------------------------------------------
        //  Bind Set
        //---------------------------------------------------------------------------

        // Bind the descriptor set to the given Command Buffer. The Set-Number will automatically found in the pipelineLayout if present.
        void bind(VkCommandBuffer cmd, PipelineLayout* pipelineLayout);

        // Bind a bunch of descriptor-sets together
        static void bind(VkCommandBuffer cmd, const std::vector<DescriptorSet*>& sets, PipelineLayout* pipelineLayout);

    protected:
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

        VkDevice                device;         // Handle to the device to update the set
        DescriptorSetLayout*    setLayout;      // The Layout from which this set was allocated from

        // Separate structs for a uniform-buffer-binding and a DYNAMIC uniform-buffer-binding
        struct UniformBufferSuper
        {
            uint8_t *pData = nullptr;
            VkDescriptorBufferInfo  bufferInfo;

            virtual ~UniformBufferSuper() {}

            void updateData(void* data, const VkDeviceSize& bufferSize = 0, const std::size_t& offset = 0, const uint32_t& dstBinding = 0);
        };

        // Represents a "normal" uniform-buffer binding (real allocated GPU memory)
        struct UniformBuffer : public UniformBufferSuper
        {
            std::unique_ptr<VulkanBuffer> buffer;
        };

        // Represents a dynamic uniform-buffer with a reference to the data-ptr on the gpu from the "parent-uniform-buffer"
        struct DynamicUniformBuffer : public UniformBufferSuper
        {
            uint32_t                dynamicOffset;
        };

        // Create a VkBuffer (and allocate memory) and update the descriptor-set for every Uniform-Buffer-Binding in the set-layout.
        void createUniformBuffers();

        // Create an normal host-visible buffer for a binding and updates the set
        void createBuffer(uint32_t dstBinding, VkDeviceSize bufferSize, uint32_t objectCount = 1);

        // Stores all uniform-buffer-data. KEY: binding-value
        std::map<int, UniformBufferSuper*> bufferBindings;

        // Stores all dynamic-offsets for use in the bind()-function.
        std::vector<uint32_t>   dynamicOffsets;

        // Basically creates an normal buffer, large enough for all sets and add some information to the dynamicSetManager
        void createDynamicUniformBuffer(uint32_t dstBinding, VkDeviceSize bufferSize);

        //---------------------------------------------------------------------------
        //  DynamicDescriptorSetManager
        //---------------------------------------------------------------------------

        // Handles the creation of dynamic-uniform-buffers
        class DynamicDescriptorSetManager
        {
            struct DynamicUniformData
            {
                VkDescriptorBufferInfo  bufferInfo;         // Reference to the "parent-buffer"
                uint32_t                numDynamicSets;     // Number dynamic sets allocated
                uint8_t*                dataPtr;            // Data-Pointer to the GPU-memory

                // Calculate and return the dynamic offset. Exception if not enough space.
                uint32_t getDynamicOffset();
            };

        public:
            // Assign dynamic-uniform-buffer(s) to a binding-num in a set-layout
            std::map<DescriptorSetLayout*, std::map<int, std::vector<DynamicUniformData>>> dynamicUniformBuffers;

            // Add the (dynamic) uniform-buffer to this manager
            void                    add(DescriptorSetLayout* setLayout, uint32_t dstBinding, const UniformBufferSuper* uniformBuffer);

            // Check if an buffer already exists for a set-layout and binding-num
            bool                    hasBuffer(DescriptorSetLayout* setLayout, uint32_t dstBinding);

            // Check if still enough place in the dynamic buffer
            bool                    enoughSpace(DescriptorSetLayout* setLayout, uint32_t dstBinding, uint32_t setCount = 1);

            // A Dynamic-Buffer needs to know this information
            VkDescriptorBufferInfo& getBufferInfo(DescriptorSetLayout* setLayout, uint32_t dstBinding);
            uint32_t                getDynamicOffset(DescriptorSetLayout* setLayout, uint32_t dstBinding);
            uint8_t*                getDataPtr(DescriptorSetLayout* setLayout, uint32_t dstBinding);
        };

        // Handles the creation of dynamic-uniform-buffers
        static DynamicDescriptorSetManager dynamicSetManager;
    };

}

#endif // !DESCRIPTOR_SET_H_
