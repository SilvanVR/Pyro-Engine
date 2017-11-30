#ifndef SPOT_LIGHT_H_
#define SPOT_LIGHT_H_

#include "point_light.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  SpotLight - Class
    //---------------------------------------------------------------------------

    class SpotLight : public PointLight
    {
    public:
        SpotLight::SpotLight(const Color& color, float intensity, float fov, const Vec3f& attenuation = Vec3f(0.0f, 0.0f, 0.3f),
                             const Point3f& position = Point3f(), const Vec3f& direction = Vec3f(), ShadowInfo* shadowInfo = new ShadowInfo(8));
        ~SpotLight() {}

        // Update light-descriptor-set
        void update(float delta) override;

        // Setter's
        void set(const Color& color, float intensity, const Vec3f& attenuation, const Point3f& position, const Vec3f& direction, float cutoff);
        void setDirection(const Vec3f& direction) { getTransform().rotation = Quatf::lookRotation(direction); }
        void setFOV(float fov);

        // Getter's
        Vec3f getDirection() { return getWorldRotation().getForward(); }
        float getFOV() { return Mathf::rad2Deg(acos(cutoff) * 2); }
        float getCutoff() { return cutoff; }

    private:
        // forbid copy and copy assignment
        SpotLight(const SpotLight& light);
        SpotLight& operator=(const SpotLight& light) {};

        float cutoff;

        // Calculate the scale for the light-volume (cone). Could not get it to work properly
        //void calculateConeScale();

        // Update light-data in MappedValues
        void updateLightData();
    };

}




#endif // !SPOT_LIGHT_H_
