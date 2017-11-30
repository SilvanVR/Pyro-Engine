#include "spot_light.h"

#include "vulkan-core/resource_manager/resource_manager.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    SpotLight::SpotLight(const Color& color, float intensity, float fov, const Vec3f& attenuation,
                         const Point3f& position, const Vec3f& direction, ShadowInfo* shadowInfo)
        : PointLight(EType::SpotLight, color, intensity, attenuation,
                     Transform(position, Vec3f(), Quatf::lookRotation(direction)), shadowInfo), 
                     cutoff(cos(Mathf::deg2Rad(fov) / 2))
    {
        // Create a descriptor-set for this light
        createDescriptorSets("DescriptorSets#SPOTLIGHT");

        if (shadowInfo != nullptr)
        {
            // Spot-Lights use Perspective-Projection for rendering the Shadow-Map
            shadowInfo->distance = getRange();
            shadowInfo->camera = new Camera(fov, 1.0f, shadowInfo->distance, LayerMask({ LAYER_DEFAULT }));

            // Create the shadow-map tex using the color-attachment from the framebuffer
            createShadowMapTex();

            // Add the shadow-camera as a child, so it follows the light around
            addChild(shadowInfo->camera);

            // Update descriptor-set
            setTexture("shadowMap", shadowMapTex);
            setInt("renderShadows", 1);
            setFloat("minVariance", 0.0001f);
            setFloat("linStep", 0.75f);
        }
        else
        {
            // Update descriptor-set with an "empty"-texture
            setTexture("shadowMap", TEXTURE_GET(TEX_DEFAULT));
        }

        updateLightData();
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Update light-descriptor-set
    void SpotLight::update(float delta)
    {
        Light::update(delta);
        
        if(isStatic()) return;

        updateLightData();
    }

    void SpotLight::set(const Color& color, float intensity, const Vec3f& attenuation, 
                        const Point3f& position, const Vec3f& direction, float cutoff)
    {
        PointLight::set(color, intensity, attenuation, position); setDirection(direction); this->cutoff = cutoff;
    }


    void SpotLight::setFOV(float fov)
    {
        this->cutoff = cos(Mathf::deg2Rad(fov) / 2);
        if (shadowInfo != nullptr)
            shadowInfo->camera->setFOV(fov);
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    //// Calculate the scale for the light-volume (cone) ..... Does not working correctly.
    //void SpotLight::calculateConeScale()
    //{
    //    float fov = getFOV();
    //    float range = getRange();

    //    float height = (range * cutoff);
    //    float val = sin(Mathf::deg2Rad(fov) / 2) * 360;
    //    transform.scale.x() = val;
    //    transform.scale.y() = val;
    //    transform.scale.z() = range;

    //    float xyScale = height;
    //    //transform.scale = Vec3f(xyScale, xyScale, range);

    //    std::cout << "FOV: " << fov << " ---- SCALE: " << transform.scale << std::endl;
    //}


    // Update light-data in MappedValues
    void SpotLight::updateLightData()
    {
        // Update descriptor-set data
        MappedValues::setColor("spotLight.pointLight.base.color", getColor());
        MappedValues::setFloat("spotLight.pointLight.base.intensity", getIntensity());
        MappedValues::setVec3f("spotLight.pointLight.base.position", getWorldPosition());
        MappedValues::setVec3f("spotLight.pointLight.attenuation", getAttenuation());
        MappedValues::setVec3f("spotLight.direction", getDirection());
        MappedValues::setFloat("spotLight.cutoff", getCutoff());

        if (shadowsEnabled())
            setMat4f("shadowMapViewProjection", getShadowViewProjection());
    }





}