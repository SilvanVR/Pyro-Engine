#include "skybox.h"

#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/data/material/texture/cubemap.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define CUBE_1 "Cubemap1"
    #define CUBE_2 "Cubemap2"

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Skybox::Skybox(CubemapPtr cubemap, CubemapPtr secondCubemap, const std::string& shaderName)
        : Renderable("Skybox", MESH("/models/sphere.obj"), nullptr,
                     Transform(Point3f(), Vec3f(10000, 10000, 10000)), Node::Dynamic, false) // Make the skybox as big as possible
    {
        assert(cubemap != nullptr);
        setupShader(shaderName);
        addLayer(LAYER_IGNORE_RAYCASTS);
        setCubemap(cubemap);
        secondCubemap.isValid() ? setSecondCubemap(secondCubemap) : setSecondCubemap(cubemap);
        setBlendFactor(0.0f);

        // Preinitialize the FOG lower- & upper-limit & disable fog
        setFogLowerLimit(0.0f);
        setFogUpperLimit(1.0f);
        setFogDensity(0.0f);

        // Set FOG-Color and enables FOG if an "FOG"-Post-Process Shader exists
        setFogColor(Color::GREY);

        // Both cubemap use first mip-level
        setMipLevel(0, 0);
        setMipLevel(0, 1);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Change the mip-level for the given cubemap index
    void Skybox::setMipLevel(float mipLevel, uint32_t cubemapIndex)
    {
        switch (cubemapIndex)
        {
        case 0: 
            assert(mipLevel < shader->getTexture(CUBE_1)->numMips());
            shader->setFloat("mipLevel1", mipLevel); break;
        case 1: 
            assert(mipLevel < shader->getTexture(CUBE_2)->numMips());
            shader->setFloat("mipLevel2", mipLevel); break;
        default: 
            Logger::Log("Skybox::setMipLevel(): Cubemap-Index '" + std::to_string(cubemapIndex) + "' does not exist.", LOGTYPE_WARNING);
        }
    }

    // Change the FOG-Color from the skybox-shader and the FOG-Post-Process-Shader if existent
    void Skybox::setFogColor(const Color& fogColor)
    {
        shader->setColor("fogColor", fogColor);
        // If the FOG Post-Process Shader exists enable the FOG for the skybox
        if (SHADER_EXISTS("FOG"))
        {
            SHADER("FOG")->setColor("fogColor", fogColor);
            setFogUpperLimit(1.0f);
            if (SHADER("FOG")->isActive()) setFogLowerLimit(0.0f);
        }
    }

    // Set the blend factor for the two cubemaps
    void Skybox::setBlendFactor(float val)
    {
        shader->setFloat("blendFactor", val);
    }

    void Skybox::setFogUpperLimit(float val)
    {
        shader->setFloat("upperLimit", val);
    }

    void Skybox::setFogLowerLimit(float val)
    {
        shader->setFloat("lowerLimit", val);
    }

    void Skybox::setFogDensity(float val)
    {
        shader->setFloat("fogDensity", val);
    }

    // Change the first cubemap for this skybox
    void Skybox::setCubemap(CubemapPtr cubemap)
    {
        shader->setTexture(CUBE_1, cubemap);
    }

    // Change the second cubemap for this skybox
    void Skybox::setSecondCubemap(CubemapPtr cubemap)
    {
        shader->setTexture(CUBE_2, cubemap);
    }

    CubemapPtr Skybox::getCubemap()
    {
        return static_cast<CubemapPtr>(shader->getTexture(CUBE_1));
    }

    CubemapPtr Skybox::getSecondCubemap()
    {
        return static_cast<CubemapPtr>(shader->getTexture(CUBE_2));
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Creates the shader and return it
    void Skybox::setupShader(const std::string& shaderName)
    {
        // High-Priority for Skybox, so it gets rendered first after deferred-shading
        ForwardShaderParams params(shaderName, "/shaders/skybox", PipelineType::Cubemap, 1000.0f);
        shader = FORWARD_SHADER(params);
        setMaterial(MATERIAL(shader));
    }

}