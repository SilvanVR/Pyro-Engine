#ifndef FREEIMAGE_LOADER_H_
#define FREEIMAGE_LOADER_H_

#include "vulkan-core/data/material/texture/texture.h"

#ifdef FREEIMAGE_LIB

#include <freeimage/FreeImage.h>

namespace Pyro
{


    class FreeImageLoader
    {
        // Initialize FreeImage-Lib when creating this instance
        static FreeImageLoader Instance;

    public:
        FreeImageLoader() { FreeImage_Initialise(); }
        ~FreeImageLoader() { FreeImage_DeInitialise(); }

        static Texture* loadTexture(const TextureParams& params);


    private:
        static void loadToGPUAndGenerateMipMaps(Texture* tex, FIBITMAP* image, bool genMips, FREE_IMAGE_FILTER filter);

    };

}

#endif // !FREEIMAGE_LIB

#endif // !FREEIMAGE_LOADER_H_
