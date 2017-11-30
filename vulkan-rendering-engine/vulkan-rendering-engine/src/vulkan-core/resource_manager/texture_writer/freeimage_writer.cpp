#include "freeimage_writer.h"

#ifdef FREEIMAGE_LIB

#include <freeimage/FreeImage.h>
#include "file_system/vfs.h"

namespace Pyro
{

    FREE_IMAGE_FORMAT getFormat(const std::string& fileExtension);

    // Write the given pixels in a file (all common formats are supported with freeimage)
    void FreeImageWriter::writeImage(const std::string& virtualPath, const std::vector<unsigned char>& pixels,
                                     const Vec2ui& resolution, const uint32_t& bytesPerPixel)
    {
        std::string physicalPath  = VFS::resolvePhysicalPath(virtualPath);
        std::string fileExtension = FileSystem::getFileExtension(virtualPath);

        uint32_t width          = resolution.x();
        uint32_t height         = resolution.y();
        uint32_t sizeInBytes    = width * height * bytesPerPixel;
        uint32_t bitsPerPixel	= bytesPerPixel * 8;

        // Allocate a bpp-bit dib
        FIBITMAP *dib = FreeImage_Allocate(width, height, bitsPerPixel, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
        
        // This works aswell, but don't know if it works well with other formats (e.g. 24 bits per pixel)
        //BYTE* ptr = FreeImage_GetBits(dib);
        //memcpy(ptr, pixels.data(), sizeInBytes);

        // Calculate the number of bytes per pixel (3 for 24-bit or 4 for 32-bit)
        int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);
        for (unsigned y = 0; y < FreeImage_GetHeight(dib); y++) {
            BYTE *bits = FreeImage_GetScanLine(dib, y);
            for (unsigned x = 0; x < FreeImage_GetWidth(dib); x++) {
                uint32_t pixelPos = y * width * bytesPerPixel + x * bytesPerPixel;
                bits[FI_RGBA_BLUE]  = pixels[pixelPos + 0];
                bits[FI_RGBA_GREEN] = pixels[pixelPos + 1];
                bits[FI_RGBA_RED]   = pixels[pixelPos + 2];
                if(bytespp == 4)
                    bits[FI_RGBA_ALPHA] = pixels[pixelPos + 3];

                // jump to next pixel
                bits += bytespp;
            }
        }

        FreeImage_FlipVertical(dib);

        FREE_IMAGE_FORMAT fif = getFormat(fileExtension);
        if(fif == FIF_JPEG && bitsPerPixel == 32)
            Logger::Log("FreeImage supports JPEG but the rendered result is in 32 Bits! JPEG has only 24 bits", LOGTYPE_ERROR);

        FreeImage_Save(fif, dib, physicalPath.c_str());

        FreeImage_Unload(dib);
    }


    FREE_IMAGE_FORMAT getFormat(const std::string& fileExtension)
    {
        FREE_IMAGE_FORMAT fif;

        if (fileExtension == "png")
            fif = FIF_PNG;
        else if (fileExtension == "jpeg" || fileExtension == "jpg")
            fif = FIF_JPEG;
        else if (fileExtension == "bmp")
            fif = FIF_BMP;
        else if (fileExtension == "exr")
            fif = FIF_EXR;
        else if (fileExtension == "gif")
            fif = FIF_GIF;
        else if (fileExtension == "hdr")
            fif = FIF_HDR;
        else if (fileExtension == "ico")
            fif = FIF_ICO;
        else if (fileExtension == "jng")
            fif = FIF_JNG;
        else if (fileExtension == "ppm")
            fif = FIF_PPM;
        else if (fileExtension == "pgm")
            fif = FIF_PGM;
        else if (fileExtension == "pbm")
            fif = FIF_PBM;
        else if (fileExtension == "pfm")
            fif = FIF_PFM;
        else if (fileExtension == "tga")
            fif = FIF_TARGA;
        else if (fileExtension == "tiff")
            fif = FIF_TIFF;
        else if (fileExtension == "wbmp")
            fif = FIF_WBMP;
        else if (fileExtension == "webp")
            fif = FIF_WEBP;
        else if (fileExtension == "xpm")
            fif = FIF_XPM;
        else
            Logger::Log("FreeImageWriter::writeImage(): Unknown format '." + fileExtension + "'", LOGTYPE_ERROR);

        return fif;
    }


}



#endif // !FREEIMAGE_LIB