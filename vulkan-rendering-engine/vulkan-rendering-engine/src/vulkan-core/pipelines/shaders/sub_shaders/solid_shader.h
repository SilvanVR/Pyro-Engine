#ifndef SOLID_SHADER_H_
#define SOLID_SHADER_H_

#include "../shader.h"

namespace ENGINE
{


    class SolidShader : public Shader
    {

    public:
        SolidShader(const std::string& name, const std::string& filePath, PipelineType pipeline, const std::vector<ShaderStage>& shaderStages)
            : Shader(name, filePath, pipeline, shaderStages) {}
        ~SolidShader() {};

        void updateUniforms(RenderingEngine* renderer) override;
    };

}




#endif // !SOLID_SHADER_H_
