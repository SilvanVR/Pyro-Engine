#include "point_light.h"

#include "vulkan-core/scene_graph/nodes/components/colliders/sphere_collider.h"
#include "vulkan-core/sub_renderer/shadow_renderer/shadow_renderer.h"
#include "vulkan-core/resource_manager/resource_manager.h"

namespace Pyro
{

    static Mesh* sphereMesh = nullptr;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    // Constructor for derived spot-light
    PointLight::PointLight(const Light::EType& type, const Color& _color, float intensity, const Vec3f& _attenuation,
                            const Transform& trans, ShadowInfo* shadowInfo)
        : Light(type, _color, intensity, trans, shadowInfo), attenuation(_attenuation)
    {
        addComponent(new SphereCollider(1.0f));
        calculateRange();
    }

    PointLight::PointLight(const Color& color, float intensity, const Vec3f& _attenuation, const Point3f& position, ShadowInfo* shadowInfo)
        : PointLight(Light::EType::PointLight, color, intensity, _attenuation, Transform(position), shadowInfo)
    {
        if (sphereMesh == nullptr) sphereMesh = MESH(MODEL_SPHERE_DEFAULT).get();

        // Just to show a bounding-box for the light-volume. The range must be 1.0, because the collider 
        // inherits the the scale of the light which represents the light-range
        addComponent(new SphereCollider(1.0f));

        // Create a descriptor-set for this light
        createDescriptorSets("DescriptorSets#POINTLIGHT");

        if (shadowInfo != nullptr)
        {
            shadowInfo->distance = getRange();

            // We dont need a view-matrix from this camera, but the projection matrix.
            shadowInfo->camera = new Camera(90, 0.1f, shadowInfo->distance, LayerMask({LAYER_DEFAULT}));
            shadowInfo->camera->setApplyVulkanClip(false);

            // Create a shadow-cubemap for this point-light and create the texture from it
            prepareCubemap();

            // Update descriptor-set
            setTexture("shadowMap", shadowMapTex);
            setInt("renderShadows", 1);
        }
        else
        {
            // Update descriptor-set with an "empty"-texture
            setTexture("shadowMap", TEXTURE_GET(CUBEMAP_DEFAULT));
        }

        updateLightData();
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    PointLight::~PointLight() 
    {
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void PointLight::render(VkCommandBuffer cmd, Resource<Shader> shader)
    {
        // Bind Index- & Vertex-Buffer of the sphere-mesh for point-lights
        sphereMesh->bind(cmd);

        // Bind Light-Descriptor-Set
        this->bind(cmd, shader->getPipelineLayout());

        // Update model-matrix through push-constant
        shader->pushConstant(cmd, 0, sizeof(Mat4f), &getWorldMatrix());

        // Draw sphere scaled with range of the light
        sphereMesh->draw(cmd);
    }

    // Update light-descriptor-set
    void PointLight::update(float delta)
    {
        Light::update(delta);

        if(isStatic()) return;

        updateLightData();
    }

    // Setter's
    void PointLight::set(const Color& color, float intensity, const Vec3f& attenuation, const Point3f& position)
    {
        Light::set(color, intensity); 
        getTransform().scale = attenuation;
        getTransform().position = position;
    }

    // Cull this point-light against the frustum
    bool PointLight::cull(Frustum* frustum)
    {
        return frustum->checkSphere(getWorldPosition(), range);
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Create a device-local cubemap which serves as the shadowmap
    void PointLight::prepareCubemap()
    {
        uint32_t size = 1 << shadowInfo->sizeAsPowerOf10;

        // 32 bit float format for higher precision
        VkFormat format = ShadowRenderer::getColorFormat();

        shadowMapTex = ADD_RAW_TEXTURE(new Cubemap(Vec2ui(size,size), format));
    }

    // Calculate the range this light can affect
    void PointLight::calculateRange()
    {
        static const float MIN_LIGHT_STRENGTH = 256.0f / 1.0f;

        Vec3f& attenuation = getAttenuation();

        // Calculate the max range the light can affect
        float exponent = getExponent() + 0.01f;
        float linear = getLinear();
        float constant = getConstant();

        float c = constant - MIN_LIGHT_STRENGTH * getIntensity() * getColor().getMax();

        range = static_cast<float>((-linear + sqrt(linear * linear - 4 * exponent * c) / (2 * exponent)));

        // Set scale for the light-volume
        getTransform().scale = Vec3f(range, range, range);
    }

    // Update light-data in MappedValues
    void PointLight::updateLightData()
    {
        // Update descriptor-set data
        MappedValues::setColor("pointLight.base.color", getColor());
        MappedValues::setFloat("pointLight.base.intensity", getIntensity());
        MappedValues::setVec3f("pointLight.base.position", getWorldPosition());
        MappedValues::setVec3f("pointLight.attenuation", getAttenuation());
    }
 

}
