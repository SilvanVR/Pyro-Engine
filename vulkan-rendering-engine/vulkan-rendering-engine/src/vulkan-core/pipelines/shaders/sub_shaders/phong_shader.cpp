#include "phong_shader.h"

#include "vulkan-core/data/material/texture/cubemap.hpp"
#include "vulkan-core/scene_graph/scene_graph.h"
#include "vulkan-core/rendering_engine.h"

#include <assert.h>

namespace ENGINE
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    PhongShader::PhongShader(const std::string& name, const std::string& filePath, PipelineType pipeline, 
                             const std::vector<ShaderStage>& shaderStages, Renderpass* renderpass)
        : Shader(name, filePath, pipeline, shaderStages, renderpass)
    {
    }

    PhongShader::~PhongShader()
    {
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Update uniforms in this shader
    void PhongShader::updateUniforms(RenderingEngine* renderer)
    {
        // Update camera position
        setVec3f("cameraPosition", renderer->getCamera()->getWorldPosition());

        // Update Ambient-Term
        setVec3f("ambientLight", renderer->getAmbientLight());
    }


}