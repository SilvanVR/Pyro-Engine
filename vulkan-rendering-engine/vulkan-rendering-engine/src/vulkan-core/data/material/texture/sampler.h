#ifndef SAMPLER_H_
#define SAMPLER_H_

#include <memory>

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Declarations
    //---------------------------------------------------------------------------

    class Sampler;
    using SSampler = std::shared_ptr<Sampler>;

    //---------------------------------------------------------------------------
    //  Enums
    //---------------------------------------------------------------------------

    enum Filter
    {
        FILTER_LINEAR = 0,
        FILTER_NEAREST = 1
    };

    enum MipmapMode
    {
        MIPMAP_MODE_LINEAR = 0,
        MIPMAP_MODE_NEAREST = 1
    };

    enum AddressMode
    {
        ADDRESS_MODE_REPEAT = 0,
        ADDRESS_MODE_MIRRORED_REPEAT = 1,
        ADDRESS_MODE_CLAMP_TO_EDGE = 2,
        ADDRESS_MODE_CLAMP_TO_BORDER = 3
    };

    //---------------------------------------------------------------------------
    //  Sampler Class
    //---------------------------------------------------------------------------

    class Sampler
    {

    public:
        // Creates a sampler which is used for sampling a texture in a shader.
        // NOTE: Once a sampler is passed to a texture, following changes to the sampler DOES NOT affect the sampling anymore.
        // This might be added in the future but is not supported yet.
        Sampler(float maxAnisotropy = 1.0f,
                Filter minFilter = FILTER_LINEAR,
                Filter magFilter = FILTER_LINEAR,
                MipmapMode mipmapMode = MIPMAP_MODE_LINEAR,
                AddressMode addressMode = ADDRESS_MODE_REPEAT);

        // Setter's
        void setMagFilter(Filter magFilter){ this->magFilter = magFilter; }
        void setMinFilter(Filter minFilter) { this->minFilter = minFilter; }
        void setFilter(Filter magFilter, const Filter minFilter) { setMagFilter(magFilter); setMinFilter(minFilter); }
        void setMipmapMode(MipmapMode mipmapMode) { this->mipmapMode = mipmapMode; }
        void setAddressMode(AddressMode addressMode) { this->addressMode = addressMode; }
        void setMaxAnisotropy(float maxAnisotropy) { this->maxAnisotropy = maxAnisotropy; }

        // Getter's
        Filter      getMagFilter() const { return magFilter; }
        Filter      getMinFilter() const { return minFilter; }
        MipmapMode  getMipmapMode() const {return mipmapMode; }
        AddressMode getAddressMode() const { return addressMode; }
        float       getMaxAnisotropy() const { return maxAnisotropy; }

    private:
        Filter      magFilter;      // Magnification filter for the Sampler
        Filter      minFilter;      // Minification filter for the Sampler
        MipmapMode  mipmapMode;     // Mipmap mode if mipmaps are present
        AddressMode addressMode;    // Adress-Mode for all directions (U,V,W)
        float       maxAnisotropy;  // Max Anisotropic - Filtering (1.0f = OFF)

    };


}



#endif // !SAMPLER_H_

