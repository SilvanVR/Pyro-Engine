#ifndef RESOURCE_FACTORY_H_
#define RESOURCE_FACTORY_H_

#include "submanager/material_manager.h"
#include "submanager/texture_manager.h"
#include "submanager/shader_manager.h"
#include "submanager/model_manager.h"
#include "resource.hpp"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define HOT_RELOADING_INTERVAL      2000

    #define MESH                        ResourceManager::createMesh
    #define NUM_MESHES                  ResourceManager::amountOfMeshes()

    #define SHADER                      ResourceManager::createShader
    #define FORWARD_SHADER              ResourceManager::createForwardShader
    #define NUM_SHADERS                 ResourceManager::amountOfShaders()
    #define SHADER_EXISTS               ResourceManager::existsShader
    #define GET_FORWARD_SHADERS         ResourceManager::getSortedForwardShaders()

    #define TEXTURE                     ResourceManager::createTexture
    #define TEXTURE_GET                 ResourceManager::getTexture
    #define CUBEMAP                     ResourceManager::createCubemap
    #define CUBEMAP_GET                 ResourceManager::getCubemap
    #define FONT_GET_DEFAULT            ResourceManager::getDefaultFont
    #define FONT_GET                    ResourceManager::getFont
    #define FONT                        ResourceManager::createFont
    #define NUM_TEXTURES                ResourceManager::amountOfTextures()
    #define IRRADIANCEMAP               ResourceManager::createIrradianceMap
    #define PREM                        ResourceManager::createPrem

    #define WRITE_IMAGE                 ResourceManager::writeImage
    #define ADD_RAW_TEXTURE             ResourceManager::addRawTexture

    #define MATERIAL                    ResourceManager::createMaterial
    #define MATERIAL_GET                ResourceManager::getMaterial
    #define NUM_MATERIALS               ResourceManager::amountOfMaterials
    #define PBRMATERIAL                 ResourceManager::createPBRMaterial

    //---------------------------------------------------------------------------
    //  ResourceManager class
    //---------------------------------------------------------------------------

    class ResourceManager
    {
        friend class VulkanBase; // Access to init() and destroy()

    public:
        // Return a handle to a mesh object. Mesh will be loaded if not already in memory.
        // @virtualPath: The virtual or relative path to the mesh
        static MeshPtr createMesh(const std::string& virtualPath);
        static uint32_t amountOfMeshes(){ return modelManager.getAmountOfResources(); }

        // <-------------- SHADER ---------------->
        static ShaderPtr createShader(const ShaderParams& params);
        static ForwardShaderPtr createForwardShader(const ForwardShaderParams& params);
        static ShaderPtr createShader(const std::string& name);
        static bool existsShader(const std::string& name) { return shaderManager.exists(name); }
        static uint32_t amountOfShaders() { return shaderManager.getAmountOfResources(); }
        static std::vector<ForwardShaderPtr> getSortedForwardShaders(){ return shaderManager.getSortedForwardShaders(); }

        // <-------------- TEXTURES ---------------->
        static uint32_t amountOfTextures(){ return textureManager.getAmountOfResources(); }
        static TexturePtr createTexture(const TextureParams& params);
        static TexturePtr getTexture(const std::string& name);
        static CubemapPtr createCubemap(const TextureParams& params);
        static CubemapPtr getCubemap(const std::string& name);
        static FontPtr createFont(const FontParams& params);
        static FontPtr getFont(const std::string& name, uint32_t fontSize);
        static FontPtr getDefaultFont();
        static IrradianceMapPtr createIrradianceMap(uint32_t size, CubemapPtr cubemap);
        static PremPtr createPrem(uint32_t size, CubemapPtr cubemap);

        // Add a new constructed texture to the texturemanager. Only for internal engine use cases.
        static TexturePtr addRawTexture(Texture* tex);

        // <-------------- MATERIALS ---------------->
        static MaterialPtr createMaterial(ShaderPtr shader, const std::string& name = "Material#" + TS(amountOfMaterials()));
        static PBRMaterialPtr createPBRMaterial(const PBRMaterialParams& params);
        static MaterialPtr getMaterial(const std::string& name);
        static uint32_t amountOfMaterials(){ return materialManager.getAmountOfResources(); }

        // Enable/Disable hot-reloading. It checks periodically whether 
        // a file has changed on disk and reloads it if enabled.
        static void setHotReloadingEnabled(bool b);

        // Save the given pixels in a file (all common formats are supported with freeimage)
        static void writeImage(const std::string& virtualPath, const ImageData& imageData) { textureManager.writeImage(virtualPath, imageData); }

    private:
        static ModelManager     modelManager;
        static ShaderManager    shaderManager;
        static TextureManager   textureManager;
        static MaterialManager  materialManager;

        static void init();
        static void destroy();
    };


}





#endif //!RESOURCE_FACTORY_H_
