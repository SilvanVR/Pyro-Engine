#ifndef SHADER_MANAGER_H_
#define SHADER_MANAGER_H_

#include "vulkan-core/pipelines/shaders/forward_shader.h"
#include "vulkan-core/pipelines/shaders/shader.h"
#include "i_resource_submanager.hpp"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define SHADER_DESCRIPTOR_SETS  "DescriptorSets"

    #define SHADER_GBUFFER          "GBuffer"
    #define SHADER_SOLID            "Solid"
    #define SHADER_DIR_LIGHT        "DirLightShader"
    #define SHADER_POINT_LIGHT      "PointLightShader"
    #define SHADER_SPOT_LIGHT       "SpotLightShader"
    #define SHADER_FW_WIREFRAME     "Wireframe"
    #define SHADER_FW_BILLBOARD     "Billboard"

    #define SHADER_FXAA             "FXAA"
    #define SHADER_HDR_BLOOM        "HDRBloom"
    #define SHADER_DEPTH_OF_FIELD   "DepthOfField"
    #define SHADER_LIGHT_SHAFTS     "LightShafts"
    #define SHADER_FOG              "FOG"

    //---------------------------------------------------------------------------
    //  ShaderManager - Class
    //---------------------------------------------------------------------------

    class ShaderManager : public IResourceSubManager<Shader>
    {
    public:
        ShaderManager() {}
        ~ShaderManager() {}

        bool exists(const std::string& name);

        ResourceID getShader(const std::string& name);
        ResourceID createShader(const ShaderParams& params);
        ResourceID createForwardShader(const ForwardShaderParams& params);

        std::vector<ForwardShaderPtr> getSortedForwardShaders();

        // IResourceSubManager Interface
        void init() override;

    private:
        ResourceID addToResourceTable(Shader* shader) override;
    };






}



#endif //!SHADER_MANAGER_H_