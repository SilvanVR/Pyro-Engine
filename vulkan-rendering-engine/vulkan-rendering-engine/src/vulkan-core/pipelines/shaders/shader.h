#ifndef SHADER_H_
#define SHADER_H_

#include "build_options.h"

#include "vulkan-core/pipelines/pipeline_layout/pipeline_layout.h"
#include "vulkan-core/resource_manager/file_resource_object.hpp"
#include "vulkan-core/pipelines/graphics_pipeline.h"
#include "vulkan-core/data/mapped_values.h"
#include "shader_module.h"

#include <string>
#include <vector>

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class RenderingEngine;
    class Material;
    class Texture;
    class Renderpass;

    //---------------------------------------------------------------------------
    //  Structs
    //---------------------------------------------------------------------------

    // "name" :             Identificator.
    // "filePath" :         Path to the Shader-Files. A vertex shader has to have always the name "vert.spv" and the fragment "frag.spv"
    // "shader-stages" :    Which shader will be loaded. Currently only vertex and fragment-shader is supported. Vertex MUST be loaded.
    // "renderpass" :       Should be almost always NULLPTR. If it is NULLPTR, then the default renderpass will be taken from the VulkanBase-Class.
    struct ShaderParams 
    {
        std::string                 name;
        std::string                 filePath;
        PipelineType                pipelineType = PipelineType::Basic;
        std::vector<ShaderStage>    shaderStages = { ShaderStage::Vertex, ShaderStage::Fragment };
        Renderpass*                 renderpass = nullptr;

        ShaderParams(const std::string& _name, 
                     const std::string& _filePath,
                     PipelineType _pipelineType = PipelineType::Basic,
                     Renderpass* _renderpass = nullptr,
                     const std::vector<ShaderStage>& _shaderStages = { ShaderStage::Vertex, ShaderStage::Fragment })
            : name(_name), filePath(_filePath), pipelineType(_pipelineType), shaderStages(_shaderStages), renderpass(_renderpass)
        {}
    };

    //---------------------------------------------------------------------------
    //  ShaderBase Class
    //---------------------------------------------------------------------------

    class Shader : public MappedValues, public FileResourceObject
    {
        friend class GraphicsPipeline;      // Allow a GraphicsPipeline to access the shader-modules
        friend class Material;              // Allow a material to add it to this shader-class

        // All shader modules. Used to keep track of already loaded modules and reuse them if possible.
        static std::map<std::string, ShaderModule*> globalShaderModules;

    public:
        // Creates a Shader, which can be used to pass it to a "Material"-Class, which renders all objects with that material then.
        Shader(const ShaderParams& params);
        virtual ~Shader();

        // Bind the shader to the given cmd. (Bind the VkPipeline and the shader descriptor-set)
        void bind(VkCommandBuffer cmd) override;

        PipelineLayout*                 getPipelineLayout() const { return m_pipelineLayout; }
        GraphicsPipeline*               getPipeline() const { return m_pipeline; }
        const std::vector<Material*>&   getMaterials() const { return m_materials; }
        bool                            hasMaterials() const { return m_materials.size() != 0; }

        // Return all materials using this shader and bound to the current scene
        std::vector<Material*>  getMaterialsFromCurrentScene();

        // Enable / Disable this Shader completely from rendering
        bool                    isActive() const { return m_isActive; }
        void                    setActive(bool b){ m_isActive = b; }
        void                    toggleActive(){ m_isActive = !m_isActive; }

        // Push the given data in the push-constant buffer from this shader. Shaderstage is automatically found.
        void pushConstant(VkCommandBuffer cmd, uint32_t offset, uint32_t size, const void* data);

    protected:
        //forbid copy and copy assignment
        Shader(const Shader& shaderBase) = delete;
        Shader& operator=(const Shader& shaderBase) = delete;

        // A VkPipelineLayout for this shader, which describes what Uniforms (DescriptorSets + pushConstants) can be bind to it.
        PipelineLayout*                     m_pipelineLayout;
        GraphicsPipeline*                   m_pipeline;         // The pipeline this shader is using
        bool                                m_isActive;         // Used to enable/disable a shader completely from rendering
        std::vector<ShaderModule*>          m_shaderModules;    // Shader-Modules for this shader (vertex, fragment etc.)
        std::vector<Material*>              m_materials;        // All materials which uses this shader

    private:
        // Return the shader modules. Called from GraphicsPipeline-class
        std::vector<ShaderModule*> getShaderModules() { return m_shaderModules; }

        // Parse all descriptor-sets/push-constants in the shader-modules and build a pipeline-layout from it
        // Return the shader set-layout if found one
        DescriptorSetLayout* createPipelineLayout();

        // Small helper function for reusing a shader-module if possible otherwise load it
        void loadShaderModule(const std::string& filePath, const ShaderStage& shaderStage);

        //---------------------------------------------------------------------------
        //  Private FRIEND Methods
        //---------------------------------------------------------------------------

        // Called from the Material-class (friend class Material)
        void addMaterial(Material* material);
        void removeMaterial(Material* material);

        // Return the descriptor-set-layout which is the material-layout for this shader. Called in the constructor of the material-class.
        DescriptorSetLayout* getMaterialSetLayout();
};

}

#endif // !SHADER_H_
