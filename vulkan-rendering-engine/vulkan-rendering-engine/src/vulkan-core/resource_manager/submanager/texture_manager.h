#ifndef TEXTURE_MANAGER_H_
#define TEXTURE_MANAGER_H_

#include "vulkan-core/data/material/texture/texture.h"
#include "vulkan-core/data/material/texture/cubemap.h"
#include "vulkan-core/gui/font.h"
#include "i_resource_submanager.hpp"

#include <memory>

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define TEX_DEFAULT                 "DefaultTexture"
    #define TEX_DEFAULT_NORMAL          "DefaultNormalTexture"
    #define TEX_DEFAULT_DISPLACEMENT    "DefaultDisplacementTexture"
    #define TEX_DEFAULT_METALLIC        "DefaultMetallicTexture"
    #define TEX_DEFAULT_ROUGHNESS       "DefaultRoughnessTexture"
    #define TEX_DEFAULT_AO_MAP          "DefaultAOMap"
    #define TEX_DEFAULT_SUN             "DefaultSunTexture"
    #define TEX_DEFAULT_CAMERA_DIRT     "DefaultCameraDirtTexture"
    #define TEX_DEFAULT_BRDF_LUT        "DefaultBRDFLut"

    #define CUBEMAP_DEFAULT             "DefaultCubemap"
    #define CUBEMAP_DEFAULT_ENVIRONMENT "DefaultEnvironmentMap"
    #define CUBEMAP_DEFAULT_IRRADIANCE  "DefaultIrradianceMap"

    #define DEFAULT_FONT                "Arial"
    #define DEBUG_FONT                  "DebugFont"

    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class Sampler;

    //---------------------------------------------------------------------------
    //  TextureManager Class
    //---------------------------------------------------------------------------

    class TextureManager : public IResourceSubManager<Texture>
    {
    public:
        TextureManager() {}
        ~TextureManager() {}

        ResourceID createTexture(const TextureParams& params);
        ResourceID getTexture(const std::string& name);

        ResourceID createCubemap(const TextureParams& params);
        ResourceID getCubemap(const std::string& name);

        ResourceID createFont(const FontParams& params);
        ResourceID getFont(const std::string& name, uint32_t fontSize);
        ResourceID getDefaultFont();

        ResourceID createIrradianceMap(uint32_t size, CubemapPtr cubemap);
        ResourceID createPrem(uint32_t size, CubemapPtr cubemap);

        void writeImage(const std::string& virtualPath, const ImageData& imageData);
        ResourceID addRawTexture(Texture* tex) { return addToResourceTable(tex); }

        // IResourceSubManager Interface
        void init() override;
        void updateResources() override;

    private:
        // IResourceSubManager Interface
        ResourceID addToResourceTable(Texture* tex) override;
        void removeFromSceneMapper(ResourceID id) override;

        void addToTextureMapper(ResourceID id, MappingValuePtr name);
        ResourceID getIDFromTextureMapper(MappingValuePtr name);

        Texture* loadTextureFromDisk(const TextureParams& params);
    };




}



#endif // !TEXTURE_MANAGER_H_
