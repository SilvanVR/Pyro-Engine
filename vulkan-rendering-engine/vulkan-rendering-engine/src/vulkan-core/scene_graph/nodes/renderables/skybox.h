#ifndef SKYBOX_H_
#define SKYBOX_H_

#include "renderable.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class Cubemap;

    //---------------------------------------------------------------------------
    //  Skybox class
    //---------------------------------------------------------------------------

    class Skybox : public Renderable
    {
    public:
        Skybox(CubemapPtr cubemap = nullptr, CubemapPtr secondCubemap = nullptr, const std::string& shaderName = "Skybox");
        ~Skybox(){}

        float getBlendFactor() { return m_material->getFloat("BlendFactor"); }
        void  setBlendFactor(float val);
        CubemapPtr getCubemap();
        CubemapPtr getSecondCubemap();

        // Fog functions
        // Change the FOG-Color from the skybox-shader and the FOG-Post-Process-Shader if existent
        void setFogColor(const Color& fogColor);
        void setFogUpperLimit(float val);
        void setFogLowerLimit(float val);
        void setFogDensity(float val);

        void setCubemap(CubemapPtr cubemap);
        void setMipLevel(float mipLevel, uint32_t cubemapIndex = 0);
        void setSecondCubemap(CubemapPtr cubemap);

        // Never cull the skybox, because the skybox is always within the view frustum
        bool cull(Frustum* frustum) override { return true; }

    private:
        // The shader this skybox is using
        ForwardShaderPtr shader;

        // Creates the shader and return it
        void setupShader(const std::string& shaderName);
    };


}

#endif // !SKYBOX_H_
