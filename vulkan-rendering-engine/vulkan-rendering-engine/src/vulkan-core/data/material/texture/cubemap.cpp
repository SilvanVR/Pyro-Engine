#include "cubemap.h"

#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/pipelines/renderpass/renderpass.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "vulkan-core/data/mesh/mesh.h"
#include "vulkan-core/vulkan_base.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    // Load the texture from the given filename
    Cubemap::Cubemap(const TextureParams& params)
        : Texture(params)
    {
    }

    // Create a cubemap on the gpu (for internal engine use cases only)
    Cubemap::Cubemap(const Vec2ui& size, VkFormat format, uint32_t numMips, const SSampler& sampler)
        : Texture(size, format, numMips, 6, sampler)
    {
    }

    //---------------------------------------------------------------------------
    //  IrradianceMap Class
    //---------------------------------------------------------------------------

    IrradianceMap::IrradianceMap(uint32_t size, CubemapPtr cubemap)
        : Cubemap(Vec2ui(size,size), VK_FORMAT_R32G32B32A32_SFLOAT, 1,
                  SSampler(new Sampler(1.0f, FILTER_LINEAR, FILTER_LINEAR, MIPMAP_MODE_NEAREST, ADDRESS_MODE_CLAMP_TO_BORDER)))
    {
        Logger::Log("Render an irradiance map... Size: [" + TS(size) + "," + TS(size) + "]", LOGTYPE_INFO);
        auto cube = MESH(MODEL_CUBE_DEFAULT);
        VulkanImage& irradianceMap  = this->getVulkanTextureResource()->getVulkanImage();

        // Create the appropriate renderpass + shader
        Renderpass renderpass(VulkanBase::getDevice(), getFormat());
        auto irradianceShader = SHADER({ "irr", "/shaders/irradiance", PipelineType::Basic, &renderpass });
        irradianceShader->setTexture("CubemapIn", cubemap);

        Mat4f projection = Mat4f::perspective(Mathf::deg2Rad(90), 1, 0.1f, 100.0f);
        vkTools::renderCubemap(&renderpass, irradianceShader, irradianceMap, 0, [&](VkCommandBuffer cmd, Mat4f view) {
            Mat4f viewProjection = projection * view;
            irradianceShader->pushConstant(cmd, 0, sizeof(Mat4f), &viewProjection);

            cube->bind(cmd);
            cube->draw(cmd);
        });
    }

    //---------------------------------------------------------------------------
    //   Prefiltered Environment (PREM) Class
    //---------------------------------------------------------------------------

    #define PREM_SAMPLES 2048

    Prem::Prem(uint32_t size, CubemapPtr cubemap)
        : Cubemap(Vec2ui(size,size), VK_FORMAT_R16G16B16A16_SFLOAT, (uint32_t)floor(log2(size)) + 1,
            SSampler(new Sampler(1.0f, FILTER_LINEAR, FILTER_LINEAR, MIPMAP_MODE_LINEAR, ADDRESS_MODE_CLAMP_TO_BORDER)))
    {
        Logger::Log("Render a preconvoluted environment map... Size: ["+TS(size)+","+TS(size)+"]." 
                    " Num-Mips: " + TS(numMips()) + " Samples: " + TS(PREM_SAMPLES), LOGTYPE_INFO);

        auto cube = MESH(MODEL_CUBE_DEFAULT);
        VulkanImage& premMap = this->getVulkanTextureResource()->getVulkanImage();

        // Create the appropriate renderpass + shader
        Renderpass renderpass(VulkanBase::getDevice(), getFormat());
        auto premShader = SHADER({ "prem", "/shaders/prem", PipelineType::Basic, &renderpass });
        premShader->setTexture("CubemapIn", cubemap);
        premShader->setFloat("resolution", (float)size);
        premShader->setInt("numSamples", PREM_SAMPLES);

        Mat4f projection = Mat4f::perspective(Mathf::deg2Rad(90), 1, 0.1f, 100.0f);
        for (uint32_t mip = 0; mip < premMap.numMips(); mip++)
        {
            float roughness = (float)mip / (premMap.numMips()-1);
            premShader->setFloat("roughness", roughness);

            vkTools::renderCubemap(&renderpass, premShader, premMap, mip, [&](VkCommandBuffer cmd, Mat4f view) {
                Mat4f viewProjection = projection * view;
                premShader->pushConstant(cmd, 0, sizeof(Mat4f), &viewProjection);

                cube->bind(cmd);
                cube->draw(cmd);
            });
        }
    }
}