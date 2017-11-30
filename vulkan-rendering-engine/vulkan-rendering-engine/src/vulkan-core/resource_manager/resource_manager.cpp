#include "resource_manager.h"

#include "time/time.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    ModelManager    ResourceManager::modelManager;
    ShaderManager   ResourceManager::shaderManager;
    TextureManager  ResourceManager::textureManager;
    MaterialManager ResourceManager::materialManager;

    //---------------------------------------------------------------------------
    //  init() & destroy()
    //---------------------------------------------------------------------------

    void ResourceManager::init()
    {
#ifdef _WIN32
        setHotReloadingEnabled(false);
#endif
        shaderManager.init();
        modelManager.init();
        textureManager.init();
        materialManager.init();
    }

    void ResourceManager::destroy()
    {
        modelManager.destroy();
        shaderManager.destroy();
        textureManager.destroy();
        materialManager.destroy();
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // <--------------------------------- MODELS ------------------------------------->

    MeshPtr ResourceManager::createMesh(const std::string& virtualPath)
    {
        ResourceID meshID = modelManager.createMesh(virtualPath);
        return MeshPtr(meshID, &modelManager);
    }

    // <--------------------------------- SHADER ------------------------------------->

    ShaderPtr ResourceManager::createShader(const ShaderParams& params)
    {
        ResourceID shaderID = shaderManager.createShader(params);
        return ShaderPtr(shaderID, &shaderManager);
    }

    ForwardShaderPtr ResourceManager::createForwardShader(const ForwardShaderParams& params)
    {
        ResourceID shaderID = shaderManager.createForwardShader(params);
        return ForwardShaderPtr(shaderID, &shaderManager);
    }

    ShaderPtr ResourceManager::createShader(const std::string& name)
    {
        ResourceID shaderID = shaderManager.getShader(name);
        return ShaderPtr(shaderID, &shaderManager);
    }

    // <------------------------------- TEXTURES ----------------------------------->

    TexturePtr ResourceManager::createTexture(const TextureParams& params)
    {
        ResourceID texID = textureManager.createTexture(params);
        return TexturePtr(texID, &textureManager);
    }

    TexturePtr ResourceManager::getTexture(const std::string& name)
    {
        ResourceID texID = textureManager.getTexture(name);
        return TexturePtr(texID, &textureManager);
    }

    CubemapPtr ResourceManager::createCubemap(const TextureParams& params)
    {
        ResourceID cubemapID = textureManager.createCubemap(params);
        return CubemapPtr(cubemapID, &textureManager);
    }

    CubemapPtr ResourceManager::getCubemap(const std::string& name)
    {
        ResourceID cubemapID = textureManager.getCubemap(name);
        return CubemapPtr(cubemapID, &textureManager);
    }

    FontPtr ResourceManager::createFont(const FontParams& params)
    {
        ResourceID fontID = textureManager.createFont(params);
        return FontPtr(fontID, &textureManager);
    }

    FontPtr ResourceManager::getFont(const std::string& name, uint32_t fontSize)
    {
        ResourceID fontID = textureManager.getFont(name, fontSize);
        return FontPtr(fontID, &textureManager);
    }

    FontPtr ResourceManager::getDefaultFont()
    {
        ResourceID fontID = textureManager.getDefaultFont();
        return FontPtr(fontID, &textureManager);
    }

    IrradianceMapPtr ResourceManager::createIrradianceMap(uint32_t size, CubemapPtr cubemap)
    {
        ResourceID id = textureManager.createIrradianceMap(size, cubemap);
        return IrradianceMapPtr(id, &textureManager);
    }

    PremPtr ResourceManager::createPrem(uint32_t size, CubemapPtr cubemap)
    {
        ResourceID id = textureManager.createPrem(size, cubemap);
        return PremPtr(id, &textureManager);
    }

    TexturePtr  ResourceManager::addRawTexture(Texture* tex)
    {
        ResourceID id = textureManager.addRawTexture(tex);
        return TexturePtr(id, &textureManager);
    }

    // <------------------------------- MATERIALS ---------------------------------->

    MaterialPtr ResourceManager::createMaterial(ShaderPtr shader, const std::string& name)
    {
        ResourceID id = materialManager.createMaterial(shader, name);
        return MaterialPtr(id, &materialManager);
    }

    PBRMaterialPtr ResourceManager::createPBRMaterial(const PBRMaterialParams& params)
    {
        ResourceID id = materialManager.createPBRMaterial(params);
        return PBRMaterialPtr(id, &materialManager);
    }

    MaterialPtr ResourceManager::getMaterial(const std::string& name)
    {
        ResourceID id = materialManager.get(name);
        return MaterialPtr(id, &materialManager);
    }

    // <--------------------------------- MISC ------------------------------------->
    void ResourceManager::setHotReloadingEnabled(bool newState)
    { 
        static bool hotReloading = false;
        static CallbackID reloadingCallback = INVALID_CALLBACK_ID;

        hotReloading = newState;
        if (hotReloading)
        {
            reloadingCallback = Time::setInterval([] {
                Logger::Log("HotReloading: Checking if resources are up to date...", LOGTYPE_INFO, LOG_LEVEL_NOT_SO_IMPORTANT);
                modelManager.updateResources();
                //textureManager.updateResources();
            }, HOT_RELOADING_INTERVAL);
        }
        else
        {
            Time::clearCallback(reloadingCallback);
        }
    }


}