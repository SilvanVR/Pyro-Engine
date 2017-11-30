#include "gli_loader.h"

#include "file_system/vfs.h"
#include <gli/gli.hpp>

namespace Pyro
{

    // Set the appropriate vulkan format for a given GLI-Format
    VkFormat getVkFormat(gli::format format)
    {
        switch (format)
        {
        case gli::FORMAT_RGBA_BP_UNORM_BLOCK16:
        case gli::FORMAT_RGBA_DXT3_UNORM_BLOCK16:
            return VK_FORMAT_BC2_UNORM_BLOCK; break;
        case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
            return VK_FORMAT_BC3_UNORM_BLOCK; break;
        case gli::FORMAT_RGBA8_UNORM_PACK8:
            return VK_FORMAT_R8G8B8A8_UNORM; break;
        case gli::FORMAT_RG16_SFLOAT_PACK16:
            return VK_FORMAT_R16G16_SFLOAT; break;
        case gli::FORMAT_BGRA8_UNORM_PACK8:
            return VK_FORMAT_B8G8R8A8_UNORM;
        default:
            Logger::Log("GLI-LOADER USE DEFAULT FORMAT 'VK_FORMAT_R8G8B8A8_UNORM'. IF THE TEXTURE IS NOT CORRECT GO TO gli_loader.h and change/add the format", LOGTYPE_WARNING);
            return VK_FORMAT_R8G8B8A8_UNORM;
        }
    }

    Texture* GliLoader::loadTexture(const TextureParams& params)
    {
        std::string physicalPath = VFS::resolvePhysicalPath(params.filePath);

        // Load file
        gli::texture loadedTex = gli::load(physicalPath);

        if (loadedTex.empty())
            Logger::Log("GliLoader::loadTexture(): Could not load texture: " + physicalPath, LogType::LOGTYPE_ERROR);

        // Convert to correct type
        gli::texture2d tex2D(loadedTex);

        // Create texture object
        Texture* pTexture = new Texture(params);

        // Set the vulkan texture format
        pTexture->m_format = getVkFormat(tex2D.format());

        // Put each relevant mip-map data into the Texture-Resource
        for (unsigned int i = 0; i < tex2D.levels(); i++)
        {
            Texture::MipMap mip{ static_cast<uint32_t>(tex2D[i].extent().x), static_cast<uint32_t>(tex2D[i].extent().y), tex2D[i].size() };
            pTexture->m_mipmaps.push_back(mip);
        }

        // Allocate memory and copy the texture data into the Texture-Resource
        uint32_t sizeInBytes = static_cast<uint32_t>(tex2D.size());
        //char* data = new char[sizeInBytes];
        //memcpy(data, tex2D.data(), sizeInBytes);

        pTexture->uploadDataToGPU(tex2D.data(), sizeInBytes);

        return pTexture;
    }

    Cubemap* GliLoader::loadCubemap(const TextureParams& params)
    {
        std::string physicalPath = VFS::resolvePhysicalPath(params.filePath);

        // Load file
        gli::texture loadedTex = gli::load(physicalPath);

        if (loadedTex.empty())
            Logger::Log("Could not load cubemap: " + physicalPath, LOGTYPE_ERROR);

        if (loadedTex.faces() <= 1)
            Logger::Log("ERROR in GliLoader::loadCubemap(): File '" + physicalPath + "' "
                "was not a cubemap! It had only one layer", LOGTYPE_ERROR);

        // Convert it to a cubemap
        gli::texture_cube texCube(loadedTex);

        // Set the vulkan texture format
        Cubemap* cubemap = new Cubemap(params);
        cubemap->m_format = getVkFormat(texCube.format());

        // Set number of cubemap faces.
        cubemap->m_layerCount = static_cast<uint32_t>(texCube.faces());

        // Allocate memory and copy the texture data into the Texture-Resource
        uint32_t sizeInBytes = static_cast<uint32_t>(texCube.size());
        char* data = new char[sizeInBytes];

        // Put each relevant mip-map data into the Texture-Resource.
        char* dataPtr = data;
        for (unsigned int level = 0; level < texCube.levels(); level++)
        {
            // MipSize in bytes
            uint32_t mipSize = static_cast<uint32_t>(texCube.size(level));
            Texture::MipMap mip{ static_cast<uint32_t>(texCube.extent(level).x), static_cast<uint32_t>(texCube.extent(level).y), mipSize };
            cubemap->m_mipmaps.push_back(mip);

            // Vulkan does expect the data in another order than GLI loads them
            // The order is: Mip-Level 0: All Faces, Mip-Level 1: All Faces etc.
            for (unsigned face = 0; face < texCube.faces(); face++)
            {
                memcpy(dataPtr, texCube.data(0, face, level), mipSize);
                dataPtr += mipSize;
            }
        }
        cubemap->uploadDataToGPU(data, sizeInBytes);
        delete data;

        return cubemap;
    }



}