#ifndef PHONG_SHADER_H_
#define PHONG_SHADER_H_

#include "../shader.h"
#include "vulkan-core/data/lighting/light.h"

namespace ENGINE
{
  
    class PhongShader : public Shader
    {
    public:
        PhongShader(const std::string& name, const std::string& filePath, PipelineType pipeline, const std::vector<ShaderStage>& shaderStages,
                    Renderpass* renderpass);
        ~PhongShader();

        void updateUniforms(RenderingEngine* renderer) override;
    };

}


#endif // !PHONG_SHADER_H_
