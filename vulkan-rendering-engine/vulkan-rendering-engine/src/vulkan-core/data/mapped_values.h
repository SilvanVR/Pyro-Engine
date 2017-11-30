#ifndef MAPPED_VALUES_H_
#define MAPPED_VALUES_H_

#include "vulkan-core/pipelines/descriptors/descriptor_set_layout.h"
#include "vulkan-core/pipelines/descriptors/descriptor_set.h"
#include "vulkan-core/data/color/color.h"
#include "data_types.hpp"

namespace Pyro
{

    class MappedValues
    {
        friend class RenderingEngine; // Access to "mappedValues"
        static std::vector<MappedValues*> mappedValues; 

    private:
        // One descriptor-set for each frame-data.
        std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;

        // A reference to the used descriptor-set-layout
        DescriptorSetLayout* descriptorSetLayout = nullptr;

        int lastFlushFrameDataIndex = -1;

        // Flush the next "numFramaDatas" descriptor-sets if something has changed CPU side.
        // Reset on every set-function call. If its zero all descriptor-sets are up to date.
        int flushCounter;

        // Send data's stored in the RAM to the GPU if necessary
        void flush(uint32_t frameDataIndex);

    protected:
        // Creates the descriptor-sets from a setName
        void createDescriptorSets(const std::string& setName);

        // Creates the descriptor-sets from a setLayout
        void createDescriptorSets(DescriptorSetLayout* setLayout);

        // True if descriptor-sets were created within this class
        bool                    hasDescriptorSets = false;

        // Delete old descriptor-sets and create new ones from the given setLayout
        void changeDescriptorSets(const std::string& setName);

        // Delete old descriptor-sets and create new ones from the given setLayout
        void changeDescriptorSets(DescriptorSetLayout* setLayout);

    public:
        MappedValues();
        MappedValues(const std::string& setName);
        ~MappedValues();

        // Bind this descriptor-set to a command-buffer if it has a descriptor-set.
        // Override in subclass, if the subclass knows the pipeline-layout, otherwise use the other bind-method.
        virtual void bind(VkCommandBuffer cmd);

        // Bind this descriptor-set to a command-buffer with the given pipeline-layout.
        // If the subclass has overriden the other bind(VkCommandBuffer cmd) - method, use that instead.
        void bind(VkCommandBuffer cmd, PipelineLayout* pipelineLayout);

        // Return the descriptor-set this shader is using - still needed?
        //DescriptorSet*          getDescriptorSet();

        // Return the descriptor-set layout - still neded?
        //DescriptorSetLayout*    getDescriptorSetLayout(){ return descriptorSetLayout; }

        // Return a reference of a used texture
        TexturePtr              getTexture(const std::string& name);
        // Return a color for a given KEY. Assert if not present.
        Color                   getColor(const std::string& name);
        // Return a int value for a given KEY. Assert if not present.
        int                     getInt(const std::string& name);
        // Return a float value for a given KEY. Assert if not present.
        float                   getFloat(const std::string& name);
        // Return a vec2 value for a given KEY. Assert if not present.
        const Vec2f&            getVec2f(const std::string& name);
        // Return a vec3 value for a given KEY. Assert if not present.
        const Vec3f&            getVec3f(const std::string& name);
        // Return a vec4 value for a given KEY. Assert if not present.
        const Vec4f&            getVec4f(const std::string& name);
        // Return a mat4f value for a given KEY. Assert if not present.
        const Mat4f&            getMat4f(const std::string& name);

        // Change the used texture. If texture = nullptr, the default texture will be applied for that channel.
        void                    setTexture(const std::string& name, TexturePtr texture, const uint32_t& dstArrayElement = 0);
        // Set a color-value in this descriptor-set.
        void                    setColor(const std::string& name, const Color& color);
        // Set a int-value in this descriptor-set.
        void                    setInt(const std::string& name, int val);
        // Set a float-value in this descriptor-set.
        void                    setFloat(const std::string& name, float val);
        // Set a vec2f in this descriptor-set.
        void                    setVec2f(const std::string& name, const Vec2f& vec);
        // Set a vec3f in this descriptor-set.
        void                    setVec3f(const std::string& name, const Vec3f& vec);
        // Set a vec4f in this descriptor-set.
        void                    setVec4f(const std::string& name, const Vec4f& vec);
        // Set a mat4f in this descriptor-set.
        void                    setMat4f(const std::string& name, const Mat4f& mat);

        // Update a uniform-buffer explicitly with the given data, offset and size.
        // HAS TO BE CALLED EVERY FRAME IF USED. Not used anymore.
        //void                    setUniformBuffer(const std::string& name, void* data, const std::size_t& offset = 0, const std::size_t& bufferSize = 0);
        // Change the used texture with an image-info. Only for special use cases.
        // HAS TO BE CALLED EVERY FRAME IF USED.
        void                    setTexture(const std::string& name, const VkDescriptorImageInfo& imageInfo, const uint32_t& dstArrayElement = 0);
        // Change the used texture with an image-info. Only for special use cases.
        // "Preinitializes" the descriptor-set specified by "index".
        void                    setTexture(int index, const std::string& name, const VkDescriptorImageInfo& imageInfo);

        // Try to find the given "name" in the descriptor-set-layout and return the data-type for it
        DataType getDataType(const std::string& name);

    private:
        // Small helper functions for the set.. - functions
        void updateBufferData(const std::string& name, void* data, uint32_t frameDataIndex);

        // Initialize maps with name from the descriptor-set-layout
        void initMapDatas();

        // Destroy all descriptor-sets and clean-up maps ("Resets" this class)
        void reset();

        // Stores references of all used textures
        std::map<std::string, TexturePtr> textureMap;

        // Stores int data
        std::map<std::string, int>    intMap;

        // Stores float data
        std::map<std::string, float>  floatMap;

        // Stores vector data               
        std::map<std::string, Vec2f>  vec2fMap;

        // Stores vector data               
        std::map<std::string, Vec3f>  vec3fMap;

        // Stores vector data               
        std::map<std::string, Vec4f>  vec4fMap;

        // Stores matrice data              
        std::map<std::string, Mat4f>  mat4fMap;

     public:
        // Print all names in all Maps with their data-types (For DEBUGGING)
        void printAllNames();

        // Print all names with values in all Maps with their data-types (For DEBUGGING)
        void printAllNamesWithValues();
    };


}


#endif // !MAPPED_VALUES_H_

