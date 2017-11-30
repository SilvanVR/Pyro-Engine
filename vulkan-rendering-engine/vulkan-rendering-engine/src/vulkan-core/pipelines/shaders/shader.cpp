#include "shader.h"

#include "vulkan-core/memory_management/vulkan_memory_manager.h"
#include "vulkan-core/scene_graph/scene_manager.h"
#include "vulkan-core/data/material/material.h"
#include "vulkan-core/rendering_engine.h"
#include "file_system/vfs.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    // All shader modules. Used to keep track of already loaded modules and reuse them if possible.
    std::map<std::string, ShaderModule*> Shader::globalShaderModules;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Shader::Shader(const ShaderParams& params)
        : FileResourceObject(params.filePath, params.name), m_isActive(true)
    {
        if (SHADER_EXISTS(params.name))
            Logger::Log("Shader::Shader(...): Given Shader-Name: '" + params.name + "' already exists! The name MUST be unique", LOGTYPE_ERROR);

        // Check if a shader was already loaded and reuse it if possible.
        for (const auto& shaderStage : params.shaderStages)
        {
            switch (shaderStage)
            {
            case ShaderStage::Vertex:
                loadShaderModule(params.filePath + "/vert.spv", shaderStage);
                break;
            case ShaderStage::Fragment:
                loadShaderModule(params.filePath + "/frag.spv", shaderStage);
                break;
            default:
                Logger::Log("Shader::Shader(): Given Shader-Type is not supported yet", LOGTYPE_ERROR);
            }
        }

        // Create the pipeline-layout from the shader-modules and search for a shader-set-layout
        DescriptorSetLayout* shaderSetLayout = createPipelineLayout();

        // Create the descriptor-set for this shader-class if it has one
        if (shaderSetLayout != nullptr)
            createDescriptorSets(shaderSetLayout);

        // Create a VkGraphicsPipeline
        m_pipeline = GraphicsPipeline::createNewPipeline(this, params.pipelineType, params.renderpass);
    }


    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Shader::~Shader()
    {
        vkDeviceWaitIdle(VulkanBase::getDevice());
        // Delete shader-module if reference-count reaches zero
        for (auto& shaderModule : m_shaderModules)
        {
            if (shaderModule->getRefCount() == 1)
                globalShaderModules.erase(shaderModule->getFilePath());
            shaderModule->destroy();
        }
        delete m_pipelineLayout;
        delete m_pipeline;

        while(!m_materials.empty())
            m_materials.front()->setShader(nullptr);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Bind the pipeline from the shader and the shader-uniforms to the given Command Buffer
    void Shader::bind(VkCommandBuffer cmd)
    {
        // Bind the VkPipeline
        m_pipeline->bind(cmd);

        // Bind shader-Set if this shader has one
        if (hasDescriptorSets)
            MappedValues::bind(cmd, m_pipelineLayout);
    }

    void Shader::pushConstant(VkCommandBuffer cmd, uint32_t offset, uint32_t size, const void* data)
    {
        VkShaderStageFlags shaderStage = m_pipelineLayout->getPushConstantShaderStage();
        vkCmdPushConstants(cmd, m_pipelineLayout->get(), shaderStage, offset, size, data);
    }

    std::vector<Material*> Shader::getMaterialsFromCurrentScene()
    {
        std::vector<Material*> currentMaterials;
        for (auto& mat : m_materials)
        {
            bool isDefaultMaterial = !mat->getBoundScene(); // default mat is not bound to a scene
            if(isDefaultMaterial || mat->getBoundScene() == SceneManager::getCurrentScene())
                currentMaterials.push_back(mat);
        }
        return currentMaterials;
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Parse all descriptor-sets/push-constants in the shader-modules and build a pipeline-layout from it
    // Return the shader set-layout if found one
    DescriptorSetLayout* Shader::createPipelineLayout()
    {
        // Return val for this function. Set to the shader-set if found one.
        DescriptorSetLayout* shaderSetLayout = nullptr;

        // Get all Descriptor-Set-Layouts from the Shader-Modules and sort them by Set-Number (merge them if needed)
        std::map<int, DescriptorSetLayout*> sortedDescriptorSetLayouts;

        // Get all Push-Constants from the Shader-Modules
        std::map<std::string, VkPushConstantRange> pushConstants;

        for (auto& shaderModule : m_shaderModules)
        {
            std::vector<DescriptorSetLayout*>& descriptorSetLayouts = shaderModule->getDescriptorSetLayouts();
            for (auto setLayout : descriptorSetLayouts)
            {
                int setNumber = setLayout->getSetNumber();

                // Set is already present in other shader-modules, so merge them together
                if (sortedDescriptorSetLayouts.count(setNumber) != 0)
                {
                    // Merge the two set-layouts together. Add shader-bindings from the second one to the first one if not present 
                    // or add the ShaderStage-access mask if present.
                    sortedDescriptorSetLayouts[setNumber]->mergeDescriptorSetLayout(setLayout);
                }
                else {
                    // Add the descriptor-set-layout to the VulkanMemoryManager if not existent. 
                    // The SetName is the Shader-Name plus the Name from the Set-Layout divorced by a hash e.g. Phong#ViewProjection
                    std::string fullSetName = m_name + "#" + setLayout->getName();
                    VMM::addDescriptorSetLayout(fullSetName, setLayout);

                    // Descriptor-Set-Layout is not in the map, so add it
                    sortedDescriptorSetLayouts[setNumber] = setLayout;
                }

                // Check if the set is a shader-set. If so save it in this class.
                if (setLayout->isShaderSet() && shaderSetLayout == nullptr)
                    shaderSetLayout = setLayout;
            }

            // Add push-constant from the shader-stage to the vector
            std::vector<PushConstant>& modulePushConstants = shaderModule->getPushConstants();
            for (auto& pushConstant : modulePushConstants)
            {
                std::string key = pushConstant.name;
                if (pushConstants.count(key) != 0)
                {
                    // Add the Shader-Stage from the current Push-Constant to the one already in the hash-map
                    pushConstants[key].stageFlags |= pushConstant.pushConstantRange.stageFlags;
                }
                else {
                    // Push-Constant is not in map, so add it
                    pushConstants[key] = pushConstant.pushConstantRange;
                }
            }

        }

        // Final vector of descriptor-set-layouts (sorted by set-number) from all shader-modules.
        std::vector<DescriptorSetLayout*> finalDescriptorSetLayouts;

        // Iterate over every Set-Layout
        for (auto& iterator = sortedDescriptorSetLayouts.begin(); iterator != sortedDescriptorSetLayouts.end(); iterator++)
            finalDescriptorSetLayouts.push_back(iterator->second);

        // Final vector of push-constants
        std::vector<VkPushConstantRange> finalPushConstants;
        for (auto& iterator = pushConstants.begin(); iterator != pushConstants.end(); iterator++)
            finalPushConstants.push_back(iterator->second);

        // Create Pipeline-Layout from Set- and Push-Constant Layouts
        m_pipelineLayout = new PipelineLayout(VulkanBase::getDevice(), finalDescriptorSetLayouts, finalPushConstants);

        return shaderSetLayout;
    }


    // Small helper function for reusing a shader-module if possible otherwise load it
    void Shader::loadShaderModule(const std::string& virtualPath, const ShaderStage& shaderStage)
    {
        std::string physicalPath = VFS::resolvePhysicalPath(virtualPath);
        if (globalShaderModules.count(physicalPath) == 0)
        {
            // Shader doesnt exist yet, so load it.
            ShaderModule* shaderModule = new ShaderModule(VulkanBase::getDevice(), physicalPath, shaderStage);
            // Increment reference count.
            shaderModule->addReference();
            // Push it to the list
            m_shaderModules.push_back(shaderModule);
            // Add it to the map
            globalShaderModules[physicalPath] = shaderModule;
        }
        else
        {
            // Shader already exist. Used it and add a reference.
            globalShaderModules[physicalPath]->addReference();
            m_shaderModules.push_back(globalShaderModules[physicalPath]);
        }
    }


    //---------------------------------------------------------------------------
    //  Private Friend Methods
    //---------------------------------------------------------------------------

    // Called from the Material-class (friend class Material)
    void Shader::addMaterial(Material* material)
    {
        m_materials.push_back(material);
    }

    // Called from the Material-class (friend class Material)
    void Shader::removeMaterial(Material* material)
    {
        m_materials.erase(std::remove(m_materials.begin(), m_materials.end(), material), m_materials.end());
    }

    // Return the descriptor-set-layout which is the material-set-layout for this shader.
    DescriptorSetLayout* Shader::getMaterialSetLayout()
    {
        for (const auto& setLayout : m_pipelineLayout->getSetLayouts())
            if (setLayout->isMaterialSet())
                return setLayout;
        return nullptr;
    }

}