#include "light.h"

#include "vulkan-core/sub_renderer/shadow_renderer/shadow_renderer.h"
#include "vulkan-core/scene_graph/nodes/camera/camera.h"
#include "vulkan-core/scene_graph/scene_manager.h"
#include "vulkan-core/vkTools/vk_tools.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    uint32_t Light::numLights = 0;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    // Construct a new base-light. If "shadowMapSizeAsPowerOf10" is less than 2, shadows are completely disabled for this light.
    Light::Light(const Light::EType& type, const Color& color, float intensity, ShadowInfo* shadowInfo)
        : Light(type, color, intensity, Point3f(), shadowInfo)
    {}

    Light::Light(const Light::EType& _type, const Color& _color, float _intensity, const Transform& trans, ShadowInfo* _shadowInfo)
        : Node("Light#" + std::to_string(numLights++), trans, Node::EType::Dynamic), type(_type), color(_color), intensity(_intensity), shadowInfo(_shadowInfo)
    {
        SceneManager::getCurrentScene()->addLight(this);
        addLayer(LAYER_LIGHTS);

        // Create a framebuffer with a color + depth attachment
        if (shadowInfo != nullptr)
            prepareFramebuffer();
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Light::~Light()
    {
        SceneManager::getCurrentScene()->removeLight(this);
        numLights--;
        if (shadowInfo != nullptr)
        {
            delete shadowInfo;
        }
    }

    //---------------------------------------------------------------------------
    //  Public Functions
    //---------------------------------------------------------------------------

    // Shadow-Functions
    bool Light::shadowsEnabled() 
    { 
        if (shadowInfo == nullptr) { return false; } 
        return shadowInfo->shadowsEnabled; 
    }
    float Light::getShadowDistance() 
    { 
        if (shadowInfo == nullptr) return 0.0f;
        return shadowInfo->distance; 
    }
    uint32_t Light::getShadowMapDimension() 
    { 
        if (shadowInfo == nullptr) return 0;
        return 1 << shadowInfo->sizeAsPowerOf10; 
    }

    Camera* Light::getShadowCamera()
    { 
        if (shadowInfo == nullptr) return nullptr;
        return shadowInfo->camera; 
    }

    TexturePtr Light::getShadowMapTex()
    { 
        return shadowMapTex; 
    }

    Mat4f Light::getShadowViewProjection() 
    { 
        if (shadowInfo == nullptr) return Mat4f::identity;
        return shadowInfo->camera->getViewProjection(); 
    }

    void Light::setShadowBlurScale(float blurScale) 
    { 
        if (shadowInfo == nullptr) return;
        shadowInfo->setBlurScale(blurScale); 
    }

    void Light::enableShadows() 
    {
        if(shadowInfo == nullptr) return;
        shadowInfo->shadowsEnabled = true; 
        setInt("renderShadows", 1); 
    }

    void Light::disableShadows() 
    { 
        if (shadowInfo == nullptr) return;
        shadowInfo->shadowsEnabled = false; 
        setInt("renderShadows", 0); 
    }

    void Light::toggleShadows() 
    { 
        if (shadowInfo == nullptr) return;
        shadowInfo->shadowsEnabled = !shadowInfo->shadowsEnabled; 
        setInt("renderShadows", (int)shadowInfo->shadowsEnabled); 
    }

    void Light::setShadows(bool active)
    {
        if (shadowInfo == nullptr) return;
        shadowInfo->shadowsEnabled = active;
        setInt("renderShadows", (int)shadowInfo->shadowsEnabled);
    }

    void Light::setShadowDistance(float val)
    {
        if (shadowInfo == nullptr) return;
        shadowInfo->distance = val;
        shadowInfo->camera->setZFar(val);
    }

    //---------------------------------------------------------------------------
    //  Private Functions
    //---------------------------------------------------------------------------

    void Light::prepareFramebuffer()
    {
        // Framebuffer
        uint32_t shadowMapSize = 1 << shadowInfo->sizeAsPowerOf10;

        VkImageUsageFlags colorUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        // Point-Light has a slightly different usage for the color-attachment in the frame-buffer
        if(type == Light::PointLight)
            colorUsage = colorUsage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        shadowInfo->framebuffer = new Framebuffer(VulkanBase::getDevice(), ShadowRenderer::getRenderPass(), 
                                                  shadowMapSize, shadowMapSize, { colorUsage });
    }
    
    // Create the "shadowMapTex".
    void Light::createShadowMapTex(const VkImageView& view)
    {
        uint32_t shadowMapSize = 1 << shadowInfo->sizeAsPowerOf10;

        VkDescriptorImageInfo imageInfo;
        imageInfo.imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.sampler       = shadowInfo->sampler->get();

        // A Point-Light has his own image-view (cubemap)
        if(view == VK_NULL_HANDLE)
            imageInfo.imageView = shadowInfo->framebuffer->getColorView()->get();
        else
            imageInfo.imageView = view;

        // Create the texture
        shadowMapTex = ADD_RAW_TEXTURE(new Texture(shadowMapSize, shadowMapSize, imageInfo));
    }

    //---------------------------------------------------------------------------
    //  ShadowInfo - Class
    //---------------------------------------------------------------------------
    
    static IDGenerator<unsigned short> idGenerator;

    ShadowInfo::ShadowInfo(uint32_t _sizeAsPowerOf10, float blurScale, float _distance)
        : sizeAsPowerOf10(_sizeAsPowerOf10), distance(_distance)
    {
        // Create Sampler
        sampler = new VulkanSampler(VulkanBase::getDevice(), VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

        float blurStrength = 1.0f;
        if (blurScale != 0.0f)
        {
            uint32_t shadowMapSize = 1 << sizeAsPowerOf10;

            // Generate unique id for this shadowinfo-object
            id = idGenerator.generateID();

            // Used to blur the variance shadow-map horizontal in a separate framebuffer
            hBlur = new GaussianBlur9x1(ShadowRenderer::getRenderPassGaussianBlur(), 
                                        "ShadowInfo#" + std::to_string(id) + "#HBlur",
                                        shadowMapSize, shadowMapSize, true, blurStrength, blurScale);
            hBlur->setSampler(sampler);

            // Used to blur the variance shadow-map vertical back into the shadow-map framebuffer
            // We pass in 0.0f for the "resolutionModifier" to disable the framebuffer-creation
            vBlur = new GaussianBlur9x1(ShadowRenderer::getRenderPass(),
                                        "ShadowInfo#" + std::to_string(id) + "#VBlur",
                                        false, blurStrength, blurScale, 0.0f);
            vBlur->setSampler(sampler);
        }
    }

    ShadowInfo::~ShadowInfo()
    {
        idGenerator.freeID(id);
        delete framebuffer;
        delete camera;
        if (hBlur != nullptr)
        {
            delete hBlur;
            delete vBlur;
        }
        delete sampler;
    }

    void ShadowInfo::setBlurScale(float blurScale)
    {
        vBlur->getShader()->setFloat("blurScale", blurScale);
        hBlur->getShader()->setFloat("blurScale", blurScale);
    }

}