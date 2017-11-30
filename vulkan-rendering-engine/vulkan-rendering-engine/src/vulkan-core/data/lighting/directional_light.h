#ifndef DIRECTIONAL_LIGHT_H_
#define DIRECTIONAL_LIGHT_H_

#include "light.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  DirectionalLight class
    //---------------------------------------------------------------------------

    class DirectionalLight : public Light
    {
    public:
        DirectionalLight::DirectionalLight(const Color& color, float intensity, const Quatf& rotation, 
                                           ShadowInfo* shadowInfo = new ShadowInfo(10, 1.0f, 150));

        DirectionalLight::DirectionalLight(const Color& color, float intensity, const Vec3f& direction, 
                                           ShadowInfo* shadowInfo = new ShadowInfo(10, 1.0f, 150))
            : DirectionalLight(color, intensity, Quatf::lookRotation(direction), shadowInfo)
        {}

        ~DirectionalLight() {}

        void update(float delta) override;
        void render(VkCommandBuffer cmd, Resource<Shader> shader) override;

        // Setter's
        void set(const Color& color, float intensity, const Quatf& direction) { Light::set(color, intensity); getTransform().rotation = direction; }
        void setRotation(const Quatf& rotation) { getTransform().rotation = rotation; }
        void setDirection(const Vec3f& dir) { setRotation(Quatf::lookRotation(dir)); }

        // Getter's
        Vec3f getDirection() { return getTransform().rotation.getForward(); }
        Quatf getRotation() { return getTransform().rotation; }

        // Set the shadow-distance for this dir-light. Changes the size of the whole orthographic-frustum.
        void setShadowDistance(float val) override;

    private:
        // forbid copy and copy assignment
        DirectionalLight(const DirectionalLight& light);
        DirectionalLight& operator=(const DirectionalLight& light) {};

        // Update light-data in MappedValues
        void updateLightData();
    };

}





#endif // !DIRECTIONAL_LIGHT_H_


