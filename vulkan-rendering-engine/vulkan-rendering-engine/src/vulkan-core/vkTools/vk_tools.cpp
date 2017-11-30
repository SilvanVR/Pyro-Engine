#include "vk_tools.h"

#include "../vulkan_base.h"
#include "vulkan-core/pipelines/framebuffers/framebuffer.h"
#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/pipelines/renderpass/renderpass.h"
#include "vulkan-core/pipelines/shaders/shader.h"

namespace Pyro
{


    namespace vkTools
    {

        // Check enabled Layers (Instance and Device)
        void checkEnabledLayers(std::vector<const char*> layerNames, std::vector<VkLayerProperties> layerProperties)
        {
            /* Check if enabled layers are present */
            for (uint32_t i = 0; i < layerNames.size(); i++)
            {
                VkBool32 found = false;
                for (uint32_t j = 0; j < layerProperties.size(); j++)
                {
                    if (!strcmp(layerNames[i], layerProperties[j].layerName))
                    {
                        found = true;
                        Logger::Log("Enabled Vulkan-Layer: " + std::string(layerNames[i]));
                    }
                }
                if (!found)
                    Logger::Log("Could not find an enabled layer: " + std::string(layerNames[i]), LogType::LOGTYPE_ERROR);
            }
        }

        // Check enabled Extensions (Instance and Device)
        void checkEnabledExtensions(std::vector<const char*> extensionNames, std::vector<VkExtensionProperties> extensionProperties)
        {
            /* Check if enabled extensions are present */
            for (uint32_t i = 0; i < extensionNames.size(); i++)
            {
                VkBool32 found = false;
                for (uint32_t j = 0; j < extensionProperties.size(); j++)
                {
                    if (!strcmp(extensionNames[i], extensionProperties[j].extensionName))
                    {
                        found = true;
                        Logger::Log("Enabled Vulkan-Extension: " + std::string(extensionNames[i]));
                    }
                }
                if (!found)
                Logger::Log("Could not find an enabled extension: " + std::string(extensionNames[i]), LogType::LOGTYPE_ERROR);
            }
        }

        // Get the appropriate memory type
        bool getMemoryType(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
            // Search memtypes to find first index with those properties
            for (uint32_t i = 0; i < 32; i++) {
                if ((typeBits & 1) == 1) {
                    // Type is available, does it match user properties?
                    if ((VulkanBase::getGPU().memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                        *typeIndex = i;
                        return true;
                    }
                }
                typeBits >>= 1;
            }
            // No memory types matched, return failure
            return false;
        }

        // Query supported depth formats and search for the best one
        VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat)
        {
            //From Sascha Willems
            std::vector<VkFormat> depthFormats = {
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM
            };

            for (auto& format : depthFormats)
            {
                VkFormatProperties formatProps;
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
                // Format must support depth stencil attachment for optimal tiling
                if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                {
                    *depthFormat = format;
                    return true;
                }
            }

            return false;
        }

        // Return the amount of bits for the given format
        uint32_t getBytesPerPixel(const VkFormat& format)
        {
            switch (format)
            {
            case VK_FORMAT_R4G4_UNORM_PACK8:
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SNORM:
            case VK_FORMAT_R8_USCALED:
            case VK_FORMAT_R8_SSCALED:
            case VK_FORMAT_R8_UINT:
            case VK_FORMAT_R8_SINT:
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_S8_UINT:
                return 1;
            case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
            case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            case VK_FORMAT_R5G6B5_UNORM_PACK16:
            case VK_FORMAT_B5G6R5_UNORM_PACK16:
            case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
            case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
            case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SNORM:
            case VK_FORMAT_R8G8_USCALED:
            case VK_FORMAT_R8G8_SSCALED:
            case VK_FORMAT_R8G8_UINT:
            case VK_FORMAT_R8G8_SINT:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R16_UNORM:
            case VK_FORMAT_R16_SNORM:
            case VK_FORMAT_R16_USCALED:
            case VK_FORMAT_R16_SSCALED:
            case VK_FORMAT_R16_UINT:
            case VK_FORMAT_R16_SINT:
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_D16_UNORM:
                return 2;
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SNORM:
            case VK_FORMAT_R8G8B8_USCALED:
            case VK_FORMAT_R8G8B8_SSCALED:
            case VK_FORMAT_R8G8B8_UINT:
            case VK_FORMAT_R8G8B8_SINT:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_B8G8R8_UNORM:
            case VK_FORMAT_B8G8R8_SNORM:
            case VK_FORMAT_B8G8R8_USCALED:
            case VK_FORMAT_B8G8R8_SSCALED:
            case VK_FORMAT_B8G8R8_UINT:
            case VK_FORMAT_B8G8R8_SINT:
            case VK_FORMAT_B8G8R8_SRGB:
            case VK_FORMAT_D16_UNORM_S8_UINT:
                return 3;
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SNORM:
            case VK_FORMAT_R8G8B8A8_USCALED:
            case VK_FORMAT_R8G8B8A8_SSCALED:
            case VK_FORMAT_R8G8B8A8_UINT:
            case VK_FORMAT_R8G8B8A8_SINT:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SNORM:
            case VK_FORMAT_B8G8R8A8_USCALED:
            case VK_FORMAT_B8G8R8A8_SSCALED:
            case VK_FORMAT_B8G8R8A8_UINT:
            case VK_FORMAT_B8G8R8A8_SINT:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
            case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
            case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
            case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
            case VK_FORMAT_A8B8G8R8_UINT_PACK32:
            case VK_FORMAT_A8B8G8R8_SINT_PACK32:
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
            case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
            case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
            case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
            case VK_FORMAT_A2R10G10B10_UINT_PACK32:
            case VK_FORMAT_A2R10G10B10_SINT_PACK32:
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
            case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
            case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
            case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            case VK_FORMAT_A2B10G10R10_SINT_PACK32:
            case VK_FORMAT_R16G16_UNORM:
            case VK_FORMAT_R16G16_SNORM:
            case VK_FORMAT_R16G16_USCALED:
            case VK_FORMAT_R16G16_SSCALED:
            case VK_FORMAT_R16G16_UINT:
            case VK_FORMAT_R16G16_SINT:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R32_UINT:
            case VK_FORMAT_R32_SINT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return 4;
            case VK_FORMAT_R16G16B16_UNORM:
            case VK_FORMAT_R16G16B16_SNORM:
            case VK_FORMAT_R16G16B16_USCALED:
            case VK_FORMAT_R16G16B16_SSCALED:
            case VK_FORMAT_R16G16B16_UINT:
            case VK_FORMAT_R16G16B16_SINT:
            case VK_FORMAT_R16G16B16_SFLOAT:
                return 6;
            case VK_FORMAT_R16G16B16A16_UNORM:
            case VK_FORMAT_R16G16B16A16_SNORM:
            case VK_FORMAT_R16G16B16A16_USCALED:
            case VK_FORMAT_R16G16B16A16_SSCALED:
            case VK_FORMAT_R16G16B16A16_UINT:
            case VK_FORMAT_R16G16B16A16_SINT:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32G32_UINT:
            case VK_FORMAT_R32G32_SINT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R64_UINT:
            case VK_FORMAT_R64_SINT:
            case VK_FORMAT_R64_SFLOAT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return 8;
            case VK_FORMAT_R32G32B32_UINT:
            case VK_FORMAT_R32G32B32_SINT:
            case VK_FORMAT_R32G32B32_SFLOAT:
                return 12;
            case VK_FORMAT_R32G32B32A32_UINT:
            case VK_FORMAT_R32G32B32A32_SINT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_R64G64_UINT:
            case VK_FORMAT_R64G64_SINT:
            case VK_FORMAT_R64G64_SFLOAT:
                return 16;
            case VK_FORMAT_R64G64B64_UINT:
            case VK_FORMAT_R64G64B64_SINT:
            case VK_FORMAT_R64G64B64_SFLOAT:
                return 24;
            case VK_FORMAT_R64G64B64A64_UINT:
            case VK_FORMAT_R64G64B64A64_SINT:
            case VK_FORMAT_R64G64B64A64_SFLOAT:
                return 32;
            default:
                Logger::Log("vkTools::getBytesPerPixel(): Given Format '" + std::to_string(format) + "' is not known! Maybe it wasn't added.", LOGTYPE_ERROR);
            }
            return 0;
        }

        void renderCubemap(Renderpass* renderpass, Resource<Shader> shader, VulkanImage& cubemap, uint32_t mipLevel, 
                           const std::function<void(VkCommandBuffer, Mat4f)>& func)
        {
            Vec2ui size = Vec2ui(cubemap.getWidth(mipLevel), cubemap.getHeight(mipLevel));

            // Create the target framebuffer, from which the cubemap-faces will be copied
            VkImageUsageFlags fboColorUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            Framebuffer* framebuffer = new Framebuffer(VulkanBase::getDevice(), renderpass, size.x(), size.y(), { fboColorUsage });

            auto cmd = VulkanBase::getCommandPool()->allocate();
            cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

            renderCubemap(cmd.get(), renderpass, shader, framebuffer, cubemap, mipLevel, Point3f(), func);

            cmd->endSubmitAndWaitForFence(VulkanBase::getDevice(), VulkanBase::getGraphicQueue());

            delete framebuffer;
        }

        void renderCubemap(CommandBuffer* cmd, Renderpass* renderpass, Resource<Shader> shader, Framebuffer* fbo, VulkanImage& cubemap,
                           Point3f position, const std::function<void(VkCommandBuffer, Mat4f)>& func)
        {
            renderCubemap(cmd, renderpass, shader, fbo, cubemap, 0, position, func);
        }

        void renderCubemap(CommandBuffer* cmd, Renderpass* renderpass, Resource<Shader> shader, Framebuffer* fbo, VulkanImage& cubemap, uint32_t mipLevel,
                           Point3f position, const std::function<void(VkCommandBuffer, Mat4f)>& func)
        {
            VulkanImage& color = fbo->getColorImage();

            // Change image layout for all cubemap faces to transfer destination
            cmd->setImageLayout(cubemap, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevel);

            cmd->setViewport(fbo);
            cmd->setScissor(fbo);

            shader->bind(cmd->get());

            for (uint32_t i = 0; i < 6; i++)
            {
                renderpass->begin(cmd->get(), fbo);

                Mat4f viewMatrix;
                switch (i)
                {
                case 0: // POSITIVE_X
                    viewMatrix = Mat4f::view(position, Vec3f::left, Vec3f::down);
                    break;
                case 1:	// NEGATIVE_X
                    viewMatrix = Mat4f::view(position, Vec3f::right, Vec3f::down);
                    break;
                case 2:	// POSITIVE_Y
                    viewMatrix = Mat4f::view(position, Vec3f::up, Vec3f::forward);
                    break;
                case 3:	// NEGATIVE_Y
                    viewMatrix = Mat4f::view(position, Vec3f::down, Vec3f::back);
                    break;
                case 4:	// POSITIVE_Z
                    viewMatrix = Mat4f::view(position, Vec3f::forward, Vec3f::down);
                    break;
                case 5:	// NEGATIVE_Z
                    viewMatrix = Mat4f::view(position, Vec3f::back, Vec3f::down);
                    break;
                }

                func(cmd->get(), viewMatrix);

                renderpass->end(cmd->get());

                // Make sure color writes to the framebuffer are finished before using it as transfer source
                cmd->setImageLayout(color, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

                // Copy rendered color-attachment from the framebuffer to one cubemap-face specified by the face-index "i"
                cmd->copyImage(color, cubemap, i, mipLevel);
            }

            cmd->setImageLayout(cubemap, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevel);
        }

        void renderFullScreenQuad(TexturePtr tex, std::string shaderPath)
        {
            Vec2ui texSize = tex->getSize();
            VkFormat format = tex->getFormat();

            Renderpass renderpass(VulkanBase::getDevice(), format);
            ShaderParams shaderParams("", shaderPath, PipelineType::PostProcess, &renderpass);
            auto shader = SHADER(shaderParams);
            Framebuffer fb(VulkanBase::getDevice(), &renderpass, texSize.x(), texSize.y(), { tex->getVulkanTextureResource()->getView() });

            auto commandBuffer = VulkanBase::getCommandPool()->allocate();
            VkCommandBuffer cmd = commandBuffer->get();

            VulkanImage& vkImage = tex->getVulkanTextureResource()->getVulkanImage();

            commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            {
                renderpass.begin(cmd, &fb);

                commandBuffer->setViewport(&fb);
                commandBuffer->setScissor(&fb);

                shader->bind(cmd);

                vkCmdDraw(cmd, 3, 1, 0, 0);

                renderpass.end(cmd);

                commandBuffer->setImageLayout(vkImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            commandBuffer->endSubmitAndWaitForFence(VulkanBase::getDevice(), VulkanBase::getGraphicQueue());
        }

        Vec2f worldToScreenPos(const Mat4f& worldMatrix, const Mat4f& viewProjection)
        {
            Vec4f eyePos    = viewProjection * worldMatrix * Vec4f(0.0f, 0.0f, 0.0f, 1.0f);

            // If w component is less than 0 then its outside
            if (eyePos.w() < 0.0f)
                return Vec2f::INVALID;

            Vec4f ndc       = Vec4f(eyePos.x() / eyePos.w(), eyePos.y() / eyePos.w(), eyePos.z() / eyePos.w(), 1.0f);
            Vec2f screenPos = Vec2f((ndc.x() + 1) / 2.0f, (ndc.y() + 1) / 2.0f);

            return screenPos;
        }

        Vec2f worldToScreenPos(const Vec3f& worldPos, const Mat4f& viewProjection)
        {
            Vec4f eyePos = viewProjection * Mat4f::translation(worldPos) * Vec4f(0.0f, 0.0f, 0.0f, 1.0f);

            // If w component is less than 0 then its outside
            if(eyePos.w() < 0.0f)
                return Vec2f::INVALID;

            Vec4f ndc = Vec4f(eyePos.x() / eyePos.w(), eyePos.y() / eyePos.w(), eyePos.z() / eyePos.w(), 1.0f);
            Vec2f screenPos = Vec2f((ndc.x() + 1) / 2.0f, (ndc.y() + 1) / 2.0f);

            return screenPos;
        }

        Vec3f screenToWorldPos(const Vec2i& screenPos, const Mat4f& projInv, const Mat4f& viewInv)
        {
            Vec2f ndcCoords  = toNormalizedDeviceCoords(screenPos);
            Vec4f clipSpace  = Vec4f(ndcCoords.x(), ndcCoords.y(), -1.0f, 1.0f);
            Vec4f eyeSpace   = toEyeSpace(clipSpace, projInv);
            Vec3f worldSpace = toWorldSpace(eyeSpace, viewInv);

            return worldSpace;
        }

        Vec2f toNormalizedDeviceCoords(const Vec2i& screenPos)
        {
            float x = (2.0f * screenPos.x()) / static_cast<float>(Window::getWidth()) - 1.0f;
            float y = (2.0f * screenPos.y()) / static_cast<float>(Window::getHeight()) - 1.0f;

            return Vec2f(x, y);
        }

        Vec4f toEyeSpace(const Vec4f& clipCoords, const Mat4f& inverseProjection)
        {
            Vec4f eyeCoords = inverseProjection * clipCoords;

            return Vec4f(eyeCoords.x(), eyeCoords.y(), -1.0f, 0.0f);
        }

        Vec3f toWorldSpace(const Vec4f& eyeCoords, const Mat4f& inverseView)
        {
            Vec4f worldCoords = inverseView * eyeCoords;

            return  Vec3f(worldCoords).normalized();
        }


    } // !namespace vkTools

}