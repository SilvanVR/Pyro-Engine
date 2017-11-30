#ifndef FORWARD_SHADER_H_
#define FORWARD_SHADER_H_

// This class represent a Shader, which renders materials (aka objects attached to them)
// in a separate rendering-pass using the forward-rendering pipeline.

#include "shader.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Structs
    //---------------------------------------------------------------------------

    struct ForwardShaderParams : public ShaderParams
    {
        float priority;
        ForwardShaderParams(const std::string& name, const std::string& filePath,
                            PipelineType pipelineType = PipelineType::Basic, float _priority = 0.0f)
            : ShaderParams(name, filePath, pipelineType), priority(_priority)
        {}
    };

    //---------------------------------------------------------------------------
    //  ForwardShader Class
    //---------------------------------------------------------------------------

    class ForwardShader : public Shader
    {
    public:
        // Creates a Shader, which can be used to pass it to a "Material"-Class, which renders all objects with that material then.
        // "params" :   Parameters for the shader class.
        // "priority" : The higher the priority the sooner this shader will be rendered.
        ForwardShader::ForwardShader(const ForwardShaderParams& params)
            : Shader(params), m_priority(params.priority)
        {}
        ForwardShader::~ForwardShader() {}

        // Get + Set the priority of this pipeline. A higher priority means this shader gets rendered first.
        float getPriority() const { return m_priority; }
        void  setPriority(float newPriority) { m_priority = newPriority; }

    private:
        //forbid copy and copy assignment
        ForwardShader(const ForwardShader& shader) = delete;
        ForwardShader& operator=(const ForwardShader& shaderBase) = delete;

        // The priority level. Higher priority means this shader will be rendered before other ones.
        float m_priority = 0.0f;
    };



}


#endif // !FORWARD_SHADER_H_
