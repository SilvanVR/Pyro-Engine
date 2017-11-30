#ifndef CUBEMAP_H_
#define CUBEMAP_H_

#include "texture.h"
#include "data_types.hpp"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Cubemap Class
    //---------------------------------------------------------------------------

    class Cubemap : public Texture
    {
    public:
        Cubemap(const TextureParams& params);
        ~Cubemap() {};

        // Create a cubemap on the gpu (for internal engine use cases only)
        Cubemap(const Vec2ui& size, VkFormat format, uint32_t numMips = 1, const SSampler& sampler = SSampler(new Sampler()));
    };

    //---------------------------------------------------------------------------
    //  IrradianceMap Class
    //---------------------------------------------------------------------------

    // Represents a class which renders an irradiance map when it gets constructed
    class IrradianceMap : public Cubemap
    {
    public:
        IrradianceMap(uint32_t size, CubemapPtr cubemap);
        ~IrradianceMap() {};
    };

    //---------------------------------------------------------------------------
    //  Prefiltered Environment (PREM) Class
    //---------------------------------------------------------------------------

    // Represents a class which renders an irradiance map when it gets constructed
    class Prem : public Cubemap
    {
    public:
        Prem(uint32_t size, CubemapPtr cubemap);
        ~Prem() {};
    };

}



#endif // !CUBEMAP_H_