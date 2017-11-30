#ifndef LIGHT_H_
#define LIGHT_H_

#include "build_options.h"
#include "vulkan-core/pipelines/framebuffers/framebuffer.h"
#include "vulkan-core/scene_graph/nodes/camera/camera.h"
#include "vulkan-core/post_processing/post_processing.h"
#include "vulkan-core/data/material/texture/texture.h"
#include "vulkan-core/util_classes/vulkan_other.h"
#include "vulkan-core/scene_graph/nodes/node.h"
#include "vulkan-core/data/mapped_values.h"
#include "vulkan-core/data/color/color.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  ShadowInfo class
    //---------------------------------------------------------------------------

    // Information for rendering shadows
    class ShadowInfo
    {
    public:
        ShadowInfo(uint32_t sizeAsPowerOf10 = 10, float blurScale = 1.0f, float distance = 100);
        ~ShadowInfo();

        bool                shadowsEnabled = true;
        uint32_t            sizeAsPowerOf10;
        float               distance;
        Camera*             camera = nullptr;

        Framebuffer*        framebuffer = nullptr;
        VulkanSampler*      sampler = nullptr;
        unsigned short      id; // unique identifier for the shaders
        GaussianBlur9x1*    hBlur;
        GaussianBlur9x1*    vBlur;

        void setBlurScale(float blurScale);
    };

    //---------------------------------------------------------------------------
    //  Light class
    //---------------------------------------------------------------------------

    class Light : public Node, public MappedValues
    {
        friend class ShadowRenderer;

    public:
        static uint32_t numLights;

        enum EType
        {
            DirectionalLight,
            PointLight,
            SpotLight
        };

        Light(const Light::EType& type, const Color& color, float intensity, ShadowInfo* shadowInfo = nullptr);
        Light(const Light::EType& type, const Color& color, float intensity, const Transform& trans, ShadowInfo* shadowInfo = nullptr);
        virtual ~Light() = 0;

        // Virtual because, point- & spotlights have to recalculate the range parameter if the intensity has changed.
        virtual void setIntensity(float intensity) { this->intensity = intensity; }

        // Setter's
        void        set(const Color& color, float intensity) { this->color = color; this->intensity = intensity; }
        void        setColor(const Color& color) { this->color = color; }
        void        setLinStep(float val) { setFloat("linStep", val);}
        void        setMinVariance(float val) { setFloat("minVariance", val); }

        // Getter's
        Color       getColor() const { return this->color; }
        Color&      getColor() { return this->color; }
        float       getIntensity() const { return this->intensity; }

        // Return the type of this light
        Light::EType getLightType() const { return type; }

        // Shadow-Functions
        bool        shadowsEnabled();
        float       getShadowDistance();
        uint32_t    getShadowMapDimension();
        Camera*     getShadowCamera();
        TexturePtr  getShadowMapTex();
        Mat4f       getShadowViewProjection();
        void        setShadowBlurScale(float blurScale);
        void        enableShadows();
        void        disableShadows();
        void        toggleShadows();
        void        setShadows(bool active);

        // Virtual because the directional-light has a slightly different method.
        virtual void    setShadowDistance(float val);

    protected:
        // forbid copy and copy assignment
        Light(const Light& light) = delete;
        Light& operator=(const Light& light) = delete;

        Color color;
        float intensity;

        // Shadow-Info class
        ShadowInfo* shadowInfo;

        // Shadow-Map Texture. Can be used to display it on the gui.
        TexturePtr shadowMapTex;

        // Create a framebuffer for the shadow-map
        void prepareFramebuffer();

        // Create the "shadowMapTex"
        void createShadowMapTex(const VkImageView& view = VK_NULL_HANDLE);

        // The type of this light
        EType type;

    };


}



#endif // !LIGHT_H_
