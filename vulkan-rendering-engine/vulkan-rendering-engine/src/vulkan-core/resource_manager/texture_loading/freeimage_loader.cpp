#include "freeimage_loader.h"

#ifdef FREEIMAGE_LIB

#include "file_system/vfs.h"
#include <algorithm>

namespace Pyro
{

    // Declare static instance, which initializes FreeImage
    FreeImageLoader FreeImageLoader::Instance;


    Texture* FreeImageLoader::loadTexture(const TextureParams& params)
    {
        std::string physicalPath = VFS::resolvePhysicalPath(params.filePath);

        // Get the file path as a const char*
        const char* filePath = physicalPath.c_str();

        // Get the file format from the header information of the image
        FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filePath, 0);

        if (format == FIF_UNKNOWN)
        {
            // Could not get the file format from the header information, so guess the format from the filename
            format = FreeImage_GetFIFFromFilename(filePath);

            // Format still unknown, exit program
            if (format == FIF_UNKNOWN)
                Logger::Log("FreeImageLoader::loadTexture(): Given File-Format from '" + physicalPath + "' is unknown", LOGTYPE_ERROR);
        }

        //check that the plugin has reading capabilities and load the file
        FIBITMAP* image = nullptr;

        if (FreeImage_FIFSupportsReading(format))
            image = FreeImage_Load(format, filePath);
        if (!image)
            Logger::Log("FreeImageLoader::loadTexture(): Could not load the texture: " + physicalPath, LOGTYPE_ERROR);

        // Convert to FIT_BITMAP first if necessary
        FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(image);
        if (imageType != FIT_BITMAP)
        {
            FIBITMAP* convertedImage = FreeImage_ConvertToType(image, FIT_BITMAP);
            if (!convertedImage)
                Logger::Log("FreeImageLoader::loadTexture(): Could not convert the texture: " + physicalPath + " to FIT_BITMAP", LOGTYPE_ERROR);
            FreeImage_Unload(image);
            image = convertedImage;
        }

        // Convert to 32-bit color format if neccessary (with an alpha channel of 1 if none exists)
        int bpp = FreeImage_GetBPP(image);
        if (bpp != 32)
        {
            FIBITMAP* convertedImage = FreeImage_ConvertTo32Bits(image);
            if (!convertedImage)
                Logger::Log("FreeImageLoader::loadTexture(): Could not convert the texture: " + physicalPath + " to a 32-bit format", LOGTYPE_ERROR);
            FreeImage_Unload(image);
            image = convertedImage;
        }

        // Flip image on vertical axes to adapt it to vulkan
        FreeImage_FlipVertical(image);

        // Create texture object
        Texture* pTexture = new Texture(params);

        // Set vulkan format. Note that the data from FreeImage is in BGRA format.
        pTexture->m_format = VK_FORMAT_B8G8R8A8_UNORM;

        // Load the pixel data into the tex-class and generate Mip-Maps if "generateMips" = true
        loadToGPUAndGenerateMipMaps(pTexture, image, params.generateMipMaps, FILTER_BOX);

        return pTexture;
    }


    void FreeImageLoader::loadToGPUAndGenerateMipMaps(Texture* tex,  FIBITMAP* image, bool genMips, FREE_IMAGE_FILTER filter)
    {
        const std::vector<Texture::MipMap>& mipmaps = tex->m_mipmaps;

        uint32_t width      = FreeImage_GetWidth(image);
        uint32_t height     = FreeImage_GetHeight(image);
        uint32_t totalSize  = 0;

        // Generate Mip-Maps if desired
        uint32_t mipLevels = 1;
        if (genMips)
            mipLevels = (uint32_t)floor(log2(std::min(width, height))) + 1;

        // Calculate widths / height / size for the new mip-levels and add the size to the total-size
        for (unsigned int i = 0; i < mipLevels; i++)
        {
            uint32_t newWidth = uint32_t(width >> i);
            uint32_t newHeight = uint32_t(height >> i);
            uint32_t newSize = newWidth * newHeight * sizeof(int);
            totalSize += newSize;
            tex->m_mipmaps.push_back({newWidth, newHeight, newSize});
        }

        // Allocate memory for all mip-levels (size is in bytes so divide it by 4)
        int* pixels = new int[totalSize / 4];

        // Rescale the original image (create mip-levels) and store them into the pixels-array
        int* pixelData = pixels;
        FIBITMAP* rescaledImage = image;
        for (unsigned int i = 0; i < tex->m_mipmaps.size(); i++)
        {
            if (i > 0) // Dont rescale the first Mip-Level (0)
            {
                FIBITMAP* oldMipLevel = rescaledImage;
                rescaledImage = FreeImage_Rescale(rescaledImage, tex->m_mipmaps[i].width, tex->m_mipmaps[i].height, filter);
                FreeImage_Unload(oldMipLevel);
            }

            // Copy Mip-Level into the pixels-array
            memcpy(pixelData, FreeImage_GetBits(rescaledImage), tex->m_mipmaps[i].size);
            
            // Advance pointer by number of "ints"
            pixelData += (tex->m_mipmaps[i].size / 4);
        }

        FreeImage_Unload(rescaledImage);

        // Now upload data to GPU
        tex->uploadDataToGPU(pixels, totalSize);

        delete pixels;
    }


}

#endif