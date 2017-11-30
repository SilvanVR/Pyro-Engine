#ifndef POINT_LIGHT_H_
#define POINT_LIGHT_H_

#include "light.h"

namespace Pyro
{
    
    //---------------------------------------------------------------------------
    //  PointLight class
    //---------------------------------------------------------------------------

    class PointLight : public Light
    {
    protected:
        // Constructor for derived spot-light
        PointLight::PointLight(const Light::EType& type, const Color& _color, float intensity, const Vec3f& attenuation,
                               const Transform& trans, ShadowInfo* shadowInfo = new ShadowInfo(8));

    public:
        PointLight::PointLight(const Color& color, float intensity, const Vec3f& attenuation, 
                               const Point3f& position, ShadowInfo* shadowInfo = new ShadowInfo(8));
        virtual ~PointLight();

        // Update light-descriptor-set
        void update(float delta) override;
        void render(VkCommandBuffer cmd, Resource<Shader> shader) override;

        // Setter's
        void set(const Color& color, float intensity, const Vec3f& attenuation, const Point3f& position);
        void set(const Color& color, float intensity, const Point3f& position) { Light::set(color, intensity); getTransform().position = position; }
        void setAttenuation(const Vec3f& attenuation) { this->attenuation = attenuation; }
        void setPosition(const Point3f& position) { getTransform().position = position; }
        void setConstant(float constant) { this->attenuation.x() = constant; calculateRange(); setShadowDistance(getRange()); }
        void setLinear(float linear) { this->attenuation.y() = linear; calculateRange(); setShadowDistance(getRange()); }
        void setExponent(float exponent) { this->attenuation.z() = exponent; calculateRange(); setShadowDistance(getRange()); }
        void setIntensity(float intensity) override { Light::setIntensity(intensity); calculateRange(); setShadowDistance(getRange()); }

        // Getter's
        Vec3f&   getAttenuation() { return attenuation; }
        float&   getConstant() { return attenuation.x(); }
        float&   getLinear() { return attenuation.y(); }
        float&   getExponent() { return attenuation.z(); }
        float    getRange() { return range; }

        // Cull this point-light against the frustum
        bool cull(Frustum* frustum) override;

        // Create a device-local cubemap which serves as the shadowmap
        void prepareCubemap();

        // Return the cubemap-image from this point-light
        VulkanImage& getCubemapImage(){ return shadowMapTex->getVulkanTextureResource()->getVulkanImage(); }

    private:
        // forbid copy and copy assignment
        PointLight(const PointLight& light) = delete;
        PointLight& operator=(const PointLight& light) = delete;

        // Constant / Linear / Exponent
        Vec3f attenuation;

        // The range-parameter in which the light can affect other pixels. Dynamically calculated, based on the intensity and attenuation.
        float range;

        // Calculate the range this light can affect
        void calculateRange();

        // Update light-data in MappedValues
        void updateLightData();
    };

}


#endif // !POINT_LIGHT_H_
