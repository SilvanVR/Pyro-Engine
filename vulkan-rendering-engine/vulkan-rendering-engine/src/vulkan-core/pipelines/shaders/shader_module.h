/* 
*  ShaderModule-Class header file.
*  Encapsulates a VkShaderModule.
*  Load's a given Shader (in SPIR-V format).
*  "ShaderStage" is the type of this Shader (struct defined in structs.hpp)
*
*  Date:    25.04.2016
*  Creator: Silvan Hau
*/

#ifndef SHADER_MODULE_H_
#define SHADER_MODULE_H_

#include "build_options.h"
#include "vulkan-core/pipelines/descriptors/descriptor_set_layout.h"
#include "../vertex_layout/vertex_layout.h"

#include <assert.h>
#include <vector>
#include <string>
#include <map>

namespace Pyro
{
    
    //---------------------------------------------------------------------------
    //  ShaderModule class
    //---------------------------------------------------------------------------

    class ShaderModule
    {
    public:
        ShaderModule::ShaderModule(VkDevice _device, const std::string& filename, const ShaderStage& _shaderStage);

        // Destroy the VkShaderModule and set layouts
        ~ShaderModule();

        // Return the file-path for this shader-module
        const std::string& getFilePath(){ return filePath; }

        // Return the VkShaderModule
        const VkShaderModule& getModule() const { return shader; }

        // Return the Shader Stage (for the pipeline)
        const ShaderStage& getShaderStage() const { return shaderStage; }

        // Return the vertex-layout from this module. Only valid if ShaderStage == VERTEX.
        const VertexLayout& getVertexLayout() const { assert(shaderStage == ShaderStage::Vertex); return vertexLayout; }

        // Return the parsed Descriptor-Set-Layouts
        std::vector<DescriptorSetLayout*>& getDescriptorSetLayouts() { return descriptorSetLayouts; }

        // Return the parsed Push-Constants
        std::vector<PushConstant>& getPushConstants() { return pushConstants; }

        // Reference-counting for sharing the same ShaderModules.
        void addReference() { refCount++; }
        uint32_t getRefCount(){ return refCount; }
        void destroy() { refCount--; if (refCount == 0) delete this; }

    private:
        // forbid copy and copy assignment
        ShaderModule(const ShaderModule& shader) {};
        ShaderModule& operator=(const ShaderModule& shader) {};

        //The Vulkan Shader Module
        VkShaderModule shader;

        //The Type of this Shader (vertex, fragment etc.)
        ShaderStage shaderStage;

        //Need a reference to the logical device for destruction 
        VkDevice device;

        // The vertex-layout from this shader
        VertexLayout vertexLayout;

        // Descriptor-Set Layouts used in this shader-module
        std::vector<DescriptorSetLayout*> descriptorSetLayouts;

        // Push-Constant range if present
        std::vector<PushConstant> pushConstants;

        // The file-path from this shader
        std::string filePath;

        // Num Shaders referencing this shader-module
        uint32_t refCount = 0;

        // Parse the SPIR-V shader-text using spirv-cross and create appropriate descriptor-set-layouts
        void parseDescriptorSets(const std::vector<uint32_t>& spv);

    };



}

#endif // !SHADER_MODULE_H_
