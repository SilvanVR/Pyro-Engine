#include "texture_manager.h"

#include "vulkan-core/resource_manager/texture_loading/freeimage_loader.h"
#include "vulkan-core/resource_manager/texture_writer/freeimage_writer.h"
#include "vulkan-core/resource_manager/font_loading/freetype_loader.h"
#include "vulkan-core/resource_manager/texture_loading/gli_loader.h"
#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/scene_graph/scene_manager.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "logger/logger.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Static Declarations
    //---------------------------------------------------------------------------

    static int DEFAULT_TEXTURE_ID;
    static int DEFAULT_FONT_ID;
    static int DEFAULT_CUBEMAP_ID;

    //---------------------------------------------------------------------------
    //  TextureManager - Init() & Destroy()
    //---------------------------------------------------------------------------

    void TextureManager::init()
    {
        SSampler defaultSampler = std::make_shared<Sampler>(8.0f, FILTER_LINEAR, FILTER_LINEAR, MIPMAP_MODE_LINEAR, ADDRESS_MODE_REPEAT);
        Texture::setDefaultSampler(defaultSampler);

#ifdef FREETYPE_LIB
        auto defaultFont = FONT({ DEFAULT_FONT, "/fonts/arial/arial.ttf", 24 });
        DEFAULT_FONT_ID = defaultFont.getID();
        addGlobalResource(defaultFont, std::make_shared<MappingFontValue>(DEFAULT_FONT, 24));
        addGlobalResource(FONT({ DEBUG_FONT, "/fonts/arial/arial.ttf", 24 }), std::make_shared<MappingFontValue>(DEBUG_FONT,24));
#endif
        auto defaultTexture = TEXTURE("/textures/defaults/diffuse.dds");
        DEFAULT_TEXTURE_ID = defaultTexture.getID();
        addGlobalResource(defaultTexture, std::make_shared<MappingValue>(TEX_DEFAULT));
        addGlobalResource(TEXTURE("/textures/defaults/normal.dds"), std::make_shared<MappingValue>(TEX_DEFAULT_NORMAL) );
        addGlobalResource(TEXTURE("/textures/defaults/displacement.dds"), std::make_shared<MappingValue>(TEX_DEFAULT_DISPLACEMENT) );
        addGlobalResource(TEXTURE("/textures/defaults/black.dds"), std::make_shared<MappingValue>(TEX_DEFAULT_METALLIC) );
        addGlobalResource(TEXTURE("/textures/defaults/white.dds"), std::make_shared<MappingValue>(TEX_DEFAULT_ROUGHNESS) );
        addGlobalResource(TEXTURE("/textures/defaults/white.dds"), std::make_shared<MappingValue>(TEX_DEFAULT_AO_MAP) );
        addGlobalResource(TEXTURE("/textures/defaults/sun.dds"), std::make_shared<MappingValue>(TEX_DEFAULT_SUN) );
        addGlobalResource(TEXTURE("/textures/defaults/lens_dirtyglass.dds"), std::make_shared<MappingValue>(TEX_DEFAULT_CAMERA_DIRT) );

        auto defaultCubemap = CUBEMAP("/textures/cubemaps/cubemap_black.dds");
        DEFAULT_CUBEMAP_ID = defaultCubemap.getID();
        addGlobalResource(defaultCubemap, std::make_shared<MappingValue>(CUBEMAP_DEFAULT));
        addGlobalResource(CUBEMAP("/textures/cubemaps/cubemap_black.dds"), std::make_shared<MappingValue>(CUBEMAP_DEFAULT_ENVIRONMENT) );
        addGlobalResource(CUBEMAP("/textures/cubemaps/cubemap_white.dds"), std::make_shared<MappingValue>(CUBEMAP_DEFAULT_IRRADIANCE) );

        auto sampler2 = std::make_shared<Sampler>(1.0f, FILTER_LINEAR, FILTER_LINEAR, MIPMAP_MODE_NEAREST, ADDRESS_MODE_CLAMP_TO_EDGE);
        TexturePtr brdfLut = ADD_RAW_TEXTURE(new Texture(Vec2ui(512,512), VK_FORMAT_R16G16_SFLOAT, sampler2));
        addGlobalResource(brdfLut, std::make_shared<MappingValue>(TEX_DEFAULT_BRDF_LUT));
        vkTools::renderFullScreenQuad(brdfLut, "/shaders/brdf_lut");
    }

    void TextureManager::updateResources()
    {
        //int visitedResources = 0;
        //for (ResourceID i = 0; i < m_resourceTable.maxPossibleResources(); i++)
        //{
        //    Texture* res = m_resourceTable[i];
        //    if (res != nullptr)
        //    {
        //        const std::string& filePath = res->getFilePath();
        //        Logger::Log("   >>>>> " + filePath);

        //        if (filePath == "") continue;
        //        std::string physicalPath = VFS::resolvePhysicalPath(filePath);

        //        if (!FileSystem::fileExists(physicalPath))
        //            continue; // File no longer exists

        //        const SystemTime& fileTime = res->getFileTime();
        //        if (fileTime != FileSystem::getLastWrittenFileTime(physicalPath))
        //        {
        //            Logger::Log("Reloading resource " + physicalPath);
        //            // Resource has to be reloaded depending on its type

        //            Texture* pNewTexture = loadTextureFromDisk(physicalPath);
        //            m_resourceTable.exchangeData(i, pNewTexture);
        //        }

        //        // Quit loop if we checked all possible resources
        //        visitedResources++;
        //        if (visitedResources == m_resourceTable.getAmountOfResources())
        //            break;
        //    }
        //}
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    ResourceID TextureManager::createTexture(const TextureParams& params)
    {
        ResourceID texID = m_resourceTable.find([&](Texture* tex) -> bool {
            return tex->getFilePath() == params.filePath;
        });

        if (texID == RESOURCE_ID_INVALID)
        {
            Logger::Log("Loading Texture '" + params.filePath + "'", LOGTYPE_INFO);

            Texture* pTexture = loadTextureFromDisk(params);
            texID = addToResourceTable(pTexture);
        }
        addToTextureMapper(texID, std::make_shared<MappingValue>(params.name));
        return texID;
    }

    ResourceID TextureManager::getTexture(const std::string& name)
    {
        ResourceID texID = getIDFromTextureMapper(std::make_shared<MappingValue>(name));

        if (texID == RESOURCE_ID_INVALID)
        {
            Logger::Log("TextureManager::get(): Can't find texture with name '" + name + "'. "
                        "Returning default texture instead.", LOGTYPE_WARNING);
            return DEFAULT_TEXTURE_ID;
        }
        return texID;
    }

    ResourceID TextureManager::createCubemap(const TextureParams& params)
    {
        ResourceID cubemapID = m_resourceTable.find([&](Texture* tex) -> bool {
            return tex->getFilePath() == params.filePath;
        });

        if (cubemapID == RESOURCE_ID_INVALID)
        {
            Cubemap* pCubemap = nullptr;
            Logger::Log("Loading Cubemap '" + params.filePath + "'", LOGTYPE_INFO);

            std::string fileExtension = FileSystem::getFileExtension(params.filePath);
            if (fileExtension == "ktx" || fileExtension == "dds")
                pCubemap = GliLoader::loadCubemap(params);
            else
                Logger::Log("TextureManager::loadCubemap(): Could not open: '" + params.filePath + "'."
                            "File format is not supported.", LOGTYPE_ERROR);

            cubemapID = addToResourceTable(pCubemap);
        }
        addToTextureMapper(cubemapID, std::make_shared<MappingValue>(params.name));

        return cubemapID;
    }

    ResourceID TextureManager::getCubemap(const std::string& name)
    {
        ResourceID cubemapID = getIDFromTextureMapper(std::make_shared<MappingValue>(name));

        if (cubemapID == RESOURCE_ID_INVALID)
        {
            Logger::Log("TextureManager::getCubemap(): Can't find cubemap with name '" + name + "'. "
                        "Returning default cubemap instead.", LOGTYPE_WARNING);
            return DEFAULT_CUBEMAP_ID;
        }
        return cubemapID;
    }

    ResourceID TextureManager::createFont(const FontParams& params)
    {
        ResourceID fontID = m_resourceTable.find([&](Texture* tex) -> bool {
            Font* font = dynamic_cast<Font*>(tex);
            return font == nullptr ? false : (font->getFilePath() == params.filePath && font->getFontSize() == params.fontSize);
        });

        if (fontID == RESOURCE_ID_INVALID)
        {
#ifdef FREETYPE_LIB
            Logger::Log("Loading Font '" + params.filePath + "' with size '" + TS(params.fontSize) + "'", LOGTYPE_INFO);
            Font* pFont = FreetypeLoader::loadFont(params);
            fontID = addToResourceTable(pFont);
#endif
        }
        addToTextureMapper(fontID, std::make_shared<MappingFontValue>(params.name, params.fontSize));
        return fontID;
    }

    ResourceID TextureManager::getFont(const std::string& name, uint32_t fontSize)
    {
        ResourceID fontID = getIDFromTextureMapper(std::make_shared<MappingFontValue>(name, fontSize));

        if (fontID == RESOURCE_ID_INVALID)
        {
            Logger::Log("TextureManager::getFont(): Could not find font '" + name + "' with size " + TS(fontSize) +
                        ". Returning default font instead.", LOGTYPE_WARNING);
            return getDefaultFont();
        }
        return fontID;
    }

    ResourceID TextureManager::getDefaultFont()
    {
        return DEFAULT_FONT_ID;
    }

    ResourceID TextureManager::createIrradianceMap(uint32_t size, CubemapPtr cubemap)
    {
        IrradianceMap* irrMap = new IrradianceMap(size, cubemap);
        ResourceID id = addToResourceTable(irrMap);
        return id;
    }

    ResourceID TextureManager::createPrem(uint32_t size, CubemapPtr cubemap)
    {
        Prem* prem = new Prem(size, cubemap);
        ResourceID id = addToResourceTable(prem);
        return id;
    }

    void TextureManager::writeImage(const std::string& virtualPath, const ImageData& imageData)
    {
#ifdef FREEIMAGE_LIB

        Logger::Log("Saving rendered result to file: " + virtualPath);
        FreeImageWriter::writeImage(virtualPath, imageData.pixels, imageData.resolution, imageData.bytesPerPixel);
#else
        Logger::Log("ResourceManager::writeImage(): Try to write an image, but FreeImage is not included. "
                    "Did you forget FREEIMAGE_LIB as the preprocessor directive?", LOGTYPE_ERROR);
#endif
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    ResourceID TextureManager::addToResourceTable(Texture* tex)
    {
        ResourceID id = m_resourceTable.add(tex);
        Logger::Log("Map texture '" + tex->getName() + "' to ID #" + TS(id), LOGTYPE_INFO, LOG_LEVEL_NOT_IMPORTANT);
        return id;
    }

    void TextureManager::addToTextureMapper(ResourceID id, MappingValuePtr value)
    {
        if (value->toString() == "") return;

        Scene* currentScene = SceneManager::getCurrentScene();
        if (currentScene != nullptr)
        {
            bool alreadyPresent = currentScene->mapTexture(id, value) || m_globalResourceMapper.exists(value);
            if(alreadyPresent)
                Logger::Log("TextureManager::addToTextureMapper(): Value '" + value->toString() + "' already exists. "
                            "Getting this texture by name will not work. Consider using a different name.", LOGTYPE_WARNING);
        }
    }

    ResourceID TextureManager::getIDFromTextureMapper(MappingValuePtr value)
    {
        ResourceID texID = RESOURCE_ID_INVALID;

        Scene* currentScene = SceneManager::getCurrentScene();
        if (currentScene != nullptr)
            texID = currentScene->getTextureID(value);

        if (texID == RESOURCE_ID_INVALID)
            texID = m_globalResourceMapper.get(value);

        return texID;
    }

    void TextureManager::removeFromSceneMapper(ResourceID id)
    {
        Scene* currentScene = SceneManager::getCurrentScene();
        if (currentScene != nullptr)
            currentScene->removeTextureID(id);
    }

    Texture* TextureManager::loadTextureFromDisk(const TextureParams& params)
    {
        Texture* pTexture = nullptr;
        std::string fileExtension = FileSystem::getFileExtension(params.filePath);
        if (fileExtension == "ktx" || fileExtension == "dds")
            pTexture = GliLoader::loadTexture(params); // Load textures from a .ktx or .dds file (which supports offline-mipmapping)
        else
    #ifdef FREEIMAGE_LIB
            pTexture = FreeImageLoader::loadTexture(params); // Load a texture from a common file format with FreeImage (e.g. jpg, png)
    #else
            Logger::Log("Could not load: '" + params.filePath + "' Texture. File-Extension is not supported. "
                "Keep in mind that FreeImage is disabled. '.ktx' and '.dds' are always supported.", LOGTYPE_ERROR);
    #endif
        return pTexture;
    }

}