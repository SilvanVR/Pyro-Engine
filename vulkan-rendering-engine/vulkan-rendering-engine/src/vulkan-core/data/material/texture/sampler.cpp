#include "sampler.h"

#include <assert.h>

namespace Pyro
{

    Sampler::Sampler(float _maxAnisotropy, Filter _minFilter, Filter _magFilter,
                     MipmapMode _mipmapMode, AddressMode _addressMode)
        : maxAnisotropy(_maxAnisotropy), minFilter(_minFilter), magFilter(_magFilter), 
          mipmapMode(_mipmapMode), addressMode(_addressMode)
    {
        assert(maxAnisotropy != 0.0f);
    }


}