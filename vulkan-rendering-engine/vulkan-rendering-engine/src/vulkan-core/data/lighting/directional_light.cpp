#include "directional_light.h"

#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/rendering_engine.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    DirectionalLight::DirectionalLight(const Color& color, float intensity, const Quatf& rotation, ShadowInfo* shadowInfo)
        : Light(EType::DirectionalLight, color, intensity, shadowInfo)
    {
        getTransform().rotation = rotation;

        // Create a descriptor-set for this light
        createDescriptorSets("DescriptorSets#DIRECTIONALLIGHT");

        if (shadowInfo != nullptr)
        {
            // Orthographic-projection for a directional-light
            float halfShadowDistance = shadowInfo->distance / 2;
            shadowInfo->camera = new Camera(-halfShadowDistance, halfShadowDistance,
                                            -halfShadowDistance, halfShadowDistance,
                                             0.0f, shadowInfo->distance, 
                                             LayerMask({ LAYER_DEFAULT }));

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
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void DirectionalLight::render(VkCommandBuffer cmd, Resource<Shader> shader)
    {
        // Bind Descriptor-Set
        this->bind(cmd, shader->getPipelineLayout());

        // Draw fullscreen quad for each dir-light
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }

    // Adapts Light-Position of a directional-light so it covers "ShadowDistance"-area in front of the camera.
    // In other words: the frustum from the DirectionalLight moves with the camera.
    void DirectionalLight::update(float delta)
    {
        static const float DIRECTIONAL_LIGHT_NEAR_CLIP_OFFSET = 15;

        if (isStatic()) return;

        // Adapt position only if shadows enabled and the light is a dynamic
        if (shadowsEnabled())
        {
            Camera* mainCamera = RenderingEngine::getCamera();

            float halfShadowDistance = getShadowDistance() / 2;

            // The Lights-Position is moving "halfShadowDistance" forward from the eye-position and then
            // move "halfShadowDistance + nearClipOffSet" backwards in light-direction
            Point3f tempPosition = mainCamera->getTransform().position
                                   + mainCamera->getTransform().rotation.getForward() * halfShadowDistance
                                   - getTransform().rotation.getForward() * (halfShadowDistance + DIRECTIONAL_LIGHT_NEAR_CLIP_OFFSET);

            // Snap the camera-position to texel-size to fix "Shadow-Swimming" coming from the fitting-algorithm
            float worldTexelSize = (getShadowDistance() / getShadowMapDimension());

            Vec3f lightSpaceCameraPos = getTransform().rotation.conjugate() * tempPosition;
            lightSpaceCameraPos.x() = worldTexelSize * floor(lightSpaceCameraPos.x() / worldTexelSize);
            lightSpaceCameraPos.y() = worldTexelSize * floor(lightSpaceCameraPos.y() / worldTexelSize);

            getTransform().position = static_cast<Point3f>(getTransform().rotation * lightSpaceCameraPos);
        }

        // Call update on light which updates shadow-cam position
        Light::update(delta);

        updateLightData();
    }

    // Set the shadow-distance for this dir-light. Changes the size of the whole orthographic-frustum.
    void DirectionalLight::setShadowDistance(float val)
    {
        shadowInfo->distance = val;
        float halfShadowDistance = shadowInfo->distance / 2;
        shadowInfo->camera->setOrthoParams(-halfShadowDistance, halfShadowDistance,
                                           -halfShadowDistance, halfShadowDistance,
                                           0.0f, shadowInfo->distance);
    }

    // Update light-data in MappedValues
    void DirectionalLight::updateLightData()
    {
        // Update descriptor-set data
        MappedValues::setColor("directionalLight.base.color", getColor());
        MappedValues::setFloat("directionalLight.base.intensity", getIntensity());
        MappedValues::setVec3f("directionalLight.base.position", getWorldPosition());
        MappedValues::setVec3f("directionalLight.direction", getDirection());

        if (shadowsEnabled())
            setMat4f("shadowMapViewProjection", getShadowViewProjection());
    }


}