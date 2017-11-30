#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "vulkan-core/resource_manager/file_resource_object.hpp"
#include "../../vulkan_texture_resource.h"
#include "sampler.h"

#include <memory>

namespace Pyro
{

    struct TextureParams
    {
        std::string filePath    = "";
        std::string name        = "";
        SSampler sampler        = SSampler();
        bool generateMipMaps    = true;

        TextureParams() {}
        TextureParams(const char* fp) : filePath(fp) {}
        TextureParams(const std::string& filePath, SSampler sampler)
            : TextureParams(filePath, "", sampler, true) {}
        TextureParams(const std::string& _filePath, const std::string& _name = "",
                      SSampler _sampler = SSampler(), bool generateMips = true)
            : filePath(_filePath), name(_name), sampler(_sampler), generateMipMaps(generateMips) {}
    };

    //---------------------------------------------------------------------------
    //  Texture Class
    //---------------------------------------------------------------------------

    class Texture : public FileResourceObject
    {
        friend class VulkanTextureResource; // Allow this class tp access the private data fields
        friend class GliLoader;             // Allow this class to access the private data fields
        friend class FreeImageLoader;       // Allow this class to access the private data fields
        friend class FreetypeLoader;        // Allow this class to access the private data fields

        static SSampler defaultSampler;

    public:
        // Create a texture from a given width & height and a format (for internal engine use-cases only)
        Texture(const Vec2ui& size, VkFormat format, const SSampler& sampler = SSampler());
        Texture(const Vec2ui& size, VkFormat format, uint32_t numMips, uint32_t numLayers,
                const SSampler& sampler = SSampler());

        // Create a texture from a image-info and given width & height (for internal engine use-cases only)
        Texture(uint32_t width, uint32_t height, const VkDescriptorImageInfo& imageInfo);
        virtual ~Texture();

        // Get width and height from the first mipmap-level
        uint32_t getWidth() const { return m_mipmaps[0].width; }
        uint32_t getHeight() const { return m_mipmaps[0].height; }
        Vec2ui   getSize() const { return Vec2ui(getWidth(), getHeight()); }
        uint32_t numMips() const { return static_cast<uint32_t>(m_mipmaps.size()); }
        VkFormat getFormat() const { return m_format; }
        const SSampler& getSampler() const { return m_sampler; }
        float getAspecRatio() const { return static_cast<float>(getWidth()) / static_cast<float>(getHeight()); }
        VulkanTextureResource* getVulkanTextureResource() const { return m_vulkanTextureResource; }

        // Push the given data into this texture-object (on the GPU) via staging. Use only when you know what you do!
        void push(const void* data, uint32_t size = WHOLE_BUFFER_SIZE, uint32_t offset = 0) { m_vulkanTextureResource->push(data, size, offset); }

        static void setDefaultSampler(SSampler newDefaultSampler) { defaultSampler = newDefaultSampler; }

    protected:
        Texture(const TextureParams& params);

        // Create the vulkan texture resource and deletes the raw-data ptr
        void uploadDataToGPU(void* data, uint32_t size);

        // necessary data for a mipmap
        struct MipMap
        {
            uint32_t    width;
            uint32_t    height;
            std::size_t size;
        };

        VkFormat                    m_format;             // The format of this texture
        std::vector<MipMap>         m_mipmaps;            // Contains necessary data for each mipmap
        uint32_t                    m_layerCount = 1;     // Layer Count (Cubemaps)
        SSampler                    m_sampler;            // The Sampler this texture is using

        // Vulkan Information for a texture (VkBuffer, imageInfo etc)
        VulkanTextureResource* m_vulkanTextureResource = nullptr;

    private:
        //forbid copy and copy assignment
        Texture(const Texture& texture) = delete;
        Texture& operator=(const Texture& texture) = delete;

    };

}

#endif // !TEXTURE_H_