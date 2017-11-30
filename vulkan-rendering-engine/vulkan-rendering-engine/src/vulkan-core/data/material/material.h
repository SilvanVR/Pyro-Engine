#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "vulkan-core/resource_manager/resource_object.hpp"
#include "vulkan-core/resource_manager/resource.hpp"
#include "vulkan-core/pipelines/shaders/shader.h"
#include "vulkan-core/data/mapped_values.h"
#include "data_types.hpp"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Material Class
    //---------------------------------------------------------------------------

    class Material : public MappedValues, public ResourceObject
    {
        friend class GUIRenderer;           // Allow the gui-renderer to access the bind()-method
        friend class RenderingEngine;       // Allow Renderer access the bind()-method
        friend class Renderable;            // Allow the renderable-class to access the addRenderable() method
        friend class ShadowRenderer;        // Allow the ShadowRenderer-class to access the bind() - method
        friend class Shader;                // Allow the Shader to access getRenderables()
        friend class MaterialManager;       // Allow him to create new materials
    
    public:
        virtual ~Material();

        // Return the used shader from this material
        ShaderPtr getShader() const { return m_shader; }

        // Change the shader which renders this material
        void setShader(ShaderPtr shader);

    protected:
        // Create a material using a custom-shader.
        // Be aware to set all relevant fields in the material for that shader before it gets used!
        Material(ShaderPtr shader, const std::string& name = "");

    private:
        //forbid copy and copy assignment
        Material(const Material& material) = delete;
        Material& operator=(const Material& material) = delete;

        ShaderPtr                   m_shader;       // The shader this material is using.
        std::vector<Renderable*>    m_renderables;  // Renderables which using this material

        // Add or remove a renderable from this material
        void addRenderable(Renderable* renderable);
        void removeRenderable(Renderable* renderable);

        // Return all (dynamic or static) renderables which use this material
        std::vector<Renderable*>& getRenderables() { return m_renderables; }

        std::vector<Renderable*> getRenderablesFromCurrentScene();

        //---------------------------------------------------------------------------
        //  Vulkan Descriptor-Set Stuff
        //---------------------------------------------------------------------------

    public:
        // Bind this material to a command-buffer if it has a descriptor-set
        void bind(VkCommandBuffer cmd) override;
    };

}



#endif // !MATERIAL_H_
