#ifndef GLI_LOADER_H_
#define GLI_LOADER_H_

#include "vulkan-core/data/material/texture/texture.h"
#include "vulkan-core/data/material/texture/cubemap.h"

namespace Pyro
{

    class GliLoader
    {
    public:
        static Texture* loadTexture(const TextureParams& params);
        static Cubemap* loadCubemap(const TextureParams& params);
    };

}

#endif // !GLI_LOADER_H_
