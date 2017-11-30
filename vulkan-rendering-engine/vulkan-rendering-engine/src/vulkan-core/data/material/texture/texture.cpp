#include "texture.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Static Declaration
    //---------------------------------------------------------------------------

    std::shared_ptr<Sampler> Texture::defaultSampler;

    //---------------------------------------------------------------------------
    //  Public Constructors
    //---------------------------------------------------------------------------

    // Loads and create a texture with a bunch of sampler-properties.
    Texture::Texture(const TextureParams& params)
        : FileResourceObject(params.filePath, params.name)
    {
        m_sampler = params.sampler ? params.sampler : defaultSampler;
    }

    Texture::Texture(uint32_t width, uint32_t height, const VkDescriptorImageInfo& imageInfo)
        : FileResourceObject("", "Internal Raw Texture")
    {
        MipMap mipMap = { width, height, 0 };
        m_mipmaps.push_back(mipMap);

        m_vulkanTextureResource = new VulkanTextureResource(imageInfo);
    }

    Texture::Texture(const Vec2ui& size, VkFormat format, uint32_t numMips, uint32_t numLayers, const SSampler& sampler)
        : FileResourceObject("", "Internal Raw Texture"), m_format(format), m_layerCount(numLayers)
    {
        m_sampler = sampler ? sampler : defaultSampler;

        for (unsigned int i = 0; i < numMips; i++)
        {
            uint32_t w = uint32_t(size.x() >> i);
            uint32_t h = uint32_t(size.y() >> i);
            m_mipmaps.push_back({ w, h, 0 });
        }

        m_vulkanTextureResource = new VulkanTextureResource(this, nullptr, 0);
    }

    Texture::Texture(const Vec2ui& size, VkFormat format, const SSampler& sampler)
        : Texture(size, format, 1, 1, sampler)
    {}

    //---------------------------------------------------------------------------
    //  Protected Constructors
    //---------------------------------------------------------------------------

    // Empty Constructor (used by subclasses e.g. font, cubemap)
    //Texture::Texture(const SSampler& sampler, const std::string& name, const std::string& filePath)
    //    : ResourceObject(filePath, name), m_sampler(sampler)
    //{
    //}

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Texture::~Texture()
    {
        if(m_vulkanTextureResource != nullptr)
            delete m_vulkanTextureResource;
    }

    //---------------------------------------------------------------------------
    //  Protected Methods
    //---------------------------------------------------------------------------

    void Texture::uploadDataToGPU(void* data, uint32_t size)
    {
        assert(data != nullptr && size != 0);

        // Create a Vulkan-Texture-Resource (load the texture-data into gpu-memory and create a descriptor-image-info)
        m_vulkanTextureResource = new VulkanTextureResource(this, data, size);
    }

}

