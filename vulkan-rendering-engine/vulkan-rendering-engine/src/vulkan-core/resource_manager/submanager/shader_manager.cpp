#include "shader_manager.h"

#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/pipelines/shaders/forward_shader.h"
#include "vulkan-core/pipelines/shaders/shader.h"
#include "vulkan-core/vulkan_base.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  ShaderManager - Init() & Destroy()
    //---------------------------------------------------------------------------

    void ShaderManager::init()
    {
        addGlobalResource(SHADER({ SHADER_DESCRIPTOR_SETS, "/shaders/descriptor_sets", PipelineType::Basic }));
        addGlobalResource(SHADER({ SHADER_SOLID, "/shaders/solid", PipelineType::Basic }));

        const std::string shaderPath = "/shaders/deferred_rendering/pbr";
        addGlobalResource(SHADER({ SHADER_GBUFFER, shaderPath + "/mrt", PipelineType::Basic, VulkanBase::getMRTRenderpass() }));
        addGlobalResource(SHADER({ SHADER_DIR_LIGHT, shaderPath + "/dir_light", PipelineType::Light, VulkanBase::getLightRenderpass() }));
        addGlobalResource(SHADER({ SHADER_POINT_LIGHT, shaderPath + "/point_light", PipelineType::Light, VulkanBase::getLightRenderpass() }));
        addGlobalResource(SHADER({ SHADER_SPOT_LIGHT, shaderPath + "/spot_light", PipelineType::Light, VulkanBase::getLightRenderpass() }));

        // Forward-Shaders
        addGlobalResource(FORWARD_SHADER({ SHADER_FW_WIREFRAME, "/shaders/solid", PipelineType::Wireframe, 0.0f }));
        addGlobalResource(FORWARD_SHADER({ SHADER_FW_BILLBOARD, "/shaders/billboard", PipelineType::AlphaBlend, 0.0f }));
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    ResourceID ShaderManager::createShader(const ShaderParams& params)
    {
        Shader* pShader = new Shader(params);
        ResourceID id = addToResourceTable(pShader);
        return id;
    }

    ResourceID ShaderManager::createForwardShader(const ForwardShaderParams& params)
    {
        ForwardShader* pShader = new ForwardShader(params);
        ResourceID id = addToResourceTable(pShader);
        return id;
    }

    ResourceID ShaderManager::getShader(const std::string& name)
    {
        ResourceID id = m_resourceTable.find([=](Shader* shader) {
            return shader->getName() == name;
        });

        if (id == RESOURCE_ID_INVALID)
            Logger::Log("ShaderManager::getResource(): Can't find shader with name '" + name + "'", LOGTYPE_ERROR);

        return id;
    }

    bool ShaderManager::exists(const std::string& name)
    {
        ResourceID id = m_resourceTable.find([=](Shader* shader) {
            return shader->getName() == name;
        });
        return id != RESOURCE_ID_INVALID;
    }

    std::vector<ForwardShaderPtr> ShaderManager::getSortedForwardShaders()
    {
        std::vector<ResourceID> forwardShaderIDs = m_resourceTable.findAll([](Shader* shader) {
            return dynamic_cast<ForwardShader*>(shader) != nullptr;
        });

        std::vector<ForwardShaderPtr> forwardShaders;
        for (auto& id : forwardShaderIDs)
            forwardShaders.push_back(ForwardShaderPtr(id, this));

        // Sort forward-shaders by priority
        std::sort(forwardShaders.begin(), forwardShaders.end(),
            [](ForwardShaderPtr shader1, ForwardShaderPtr shader2) -> bool
        { return shader1->getPriority() > shader2->getPriority(); });

        return forwardShaders;
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    ResourceID ShaderManager::addToResourceTable(Shader* shader)
    {
        ResourceID id = m_resourceTable.add(shader);
        Logger::Log("Map shader '" + shader->getName() + "' to ID #" + TS(id), LOGTYPE_INFO, LOG_LEVEL_NOT_IMPORTANT);
        return id;
    }


}