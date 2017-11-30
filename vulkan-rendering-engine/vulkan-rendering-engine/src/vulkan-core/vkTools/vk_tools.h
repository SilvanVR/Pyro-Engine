#ifndef VK_TOOLS_H_
#define VK_TOOLS_H_

#include "build_options.h"
#include "vulkan-core/resource_manager/resource.hpp"
#include "data_types.hpp"

namespace Pyro
{

    class CommandBuffer;
    class VulkanImage;
    class Framebuffer;
    class Renderpass;
    class Camera;

    namespace vkTools
    {
        // Check enabled Layers and Extensions (Instance and Device)
        void checkEnabledLayers(std::vector<const char*> layerNames, std::vector<VkLayerProperties> layerProperties);
        void checkEnabledExtensions(std::vector<const char*> extensionNames, std::vector<VkExtensionProperties> extensionProperties);

        // Get the appropriate memory type
        bool getMemoryType(uint32_t typeBits, VkFlags requirementsMask, uint32_t *typeIndex);

        // Query supported depth formats and search for the best one
        VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat);

        // Return the amount of bits for the given format
        uint32_t getBytesPerPixel(const VkFormat& format);

        void renderCubemap(Renderpass* renderpass, Resource<Shader> shader, VulkanImage& cubemap, uint32_t mipLevel,
                           const std::function<void(VkCommandBuffer, Mat4f)>& func);
        void renderCubemap(CommandBuffer* cmd, Renderpass* renderpass, Resource<Shader> shader, Framebuffer* fbo, VulkanImage& cubemap,
                           Point3f position, const std::function<void(VkCommandBuffer, Mat4f)>& func);
        void renderCubemap(CommandBuffer* cmd, Renderpass* renderpass, Resource<Shader> shader, Framebuffer* fbo, VulkanImage& cubemap, uint32_t mipLevel,
                           Point3f position, const std::function<void(VkCommandBuffer, Mat4f)>& func);

        // Loads a shader from a file and renders a fullscreen quad in the given texture
        void renderFullScreenQuad(TexturePtr tex, std::string shaderPath);

        // Calculates from a world transform the position on screen in NDC coordinates (range 0 to 1)
        Vec2f worldToScreenPos(const Mat4f& worldMatrix, const Mat4f& viewProjection);
        Vec2f worldToScreenPos(const Vec3f& worldPos, const Mat4f& viewProjection);

        // Calculates from a position on the screen a vector in world-space
        Vec3f screenToWorldPos(const Vec2i& screenPos, const Mat4f& projInv, const Mat4f& viewInv);

        // Transform the 2d-position on the screen to normaliced-device-coordinates (range -1 to + 1)
        Vec2f toNormalizedDeviceCoords(const Vec2i& screenPos);

        // Transform the Clip-Space coordinates to eye space using the inverse projection matrix.
        Vec4f toEyeSpace(const Vec4f& clipCoords, const Mat4f& inverseProjection);

        // Transform the eye-space vector to world-space using the inverse view matrix.
        Vec3f toWorldSpace(const Vec4f& eyeCoords, const Mat4f& inverseView);

    }

    // Retrieve a list of Vk... handles from a list of Vulkan... objects
    // The Vulkan-Class does need to support the "get()" function which returns the vulkan-object
    template <typename T, typename T2>
    std::vector<T> getVkList(const std::vector<const T2*>& list)
    {
        std::vector<T> vkObjects;
        for (const auto& item : list)
            vkObjects.push_back(item->get());
        return vkObjects;
    }


}




#endif // !VK_TOOLS_H_
