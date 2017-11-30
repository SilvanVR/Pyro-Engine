#include "material.h"

#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"
#include "vulkan-core/scene_graph/scene_manager.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------

    // Create a material using a custom-shader.
    // Be aware to set all relevant fields in the material for that shader before it gets used!
    // Tutorial on how to use it in vulkan_debug.txt
    Material::Material(ShaderPtr shader, const std::string& name)
        : ResourceObject(name), m_shader(shader)
    {
        DescriptorSetLayout* descriptorSetLayout = m_shader->getMaterialSetLayout();

        if (descriptorSetLayout != nullptr)
            createDescriptorSets(descriptorSetLayout);

        m_shader->addMaterial(this);
    }


    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Material::~Material()
    {
        // Notify shader that a material has to be removed
        if(m_shader.isValid())
            m_shader->removeMaterial(this);

        // Change material from renderables which belong to this one
        for(auto& renderable : m_renderables)
            renderable->setMaterial(nullptr);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Change the shader which renders this material
    void Material::setShader(ShaderPtr newShader)
    {
        // Notify shader that a material has to be removed
        if(m_shader.isValid())
            m_shader->removeMaterial(this);

        m_shader = newShader;
        if (m_shader != nullptr)
        {
            m_shader->addMaterial(this);

            // Delete current descriptor-sets and create new ones
            changeDescriptorSets(m_shader->getMaterialSetLayout());
        }
    }

    std::vector<Renderable*> Material::getRenderablesFromCurrentScene()
    {
        if (getBoundScene() == nullptr)
        {
            // This means the material is a global material and can have renderables across several scenes
            // But because its only desired to render the renderables from the current scene this is necessary
            std::vector<Renderable*> renderables;
            for (auto& r : m_renderables)
            {
                if (r->getBoundScene() == SceneManager::getCurrentScene())
                    renderables.push_back(r);
            }
            return renderables;
        }
        else
        {
            return m_renderables;
        }
    }

    //---------------------------------------------------------------------------
    //  Private FRIEND Methods
    //---------------------------------------------------------------------------

    void Material::bind(VkCommandBuffer cmd)
    {
        if (hasDescriptorSets)
            MappedValues::bind(cmd, m_shader->getPipelineLayout());
    }

    void Material::addRenderable(Renderable* renderable)
    {
        m_renderables.push_back(renderable);
    }

    void Material::removeRenderable(Renderable* renderable)
    {
        m_renderables.erase(std::remove(m_renderables.begin(), m_renderables.end(), renderable), m_renderables.end());
    }

}