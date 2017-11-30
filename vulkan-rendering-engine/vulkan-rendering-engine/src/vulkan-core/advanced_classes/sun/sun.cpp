#include "sun.h"

#include "vulkan-core/sub_renderer/post_processing_renderer/post_processing_renderer.h"
#include "vulkan-core/pipelines/shaders/forward_shader.h"
#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/rendering_engine.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Sun::Sun(const Vec3f& direction, const Vec2f& scale, const Color& color, float intensity, ShadowInfo* shadowInfo)
        : Renderable("Sun", MESH(MODEL_QUAD_DEFAULT), nullptr,
                     Transform(Point3f(), Vec3f(scale.x(), scale.y(), 1), Quatf::lookRotation(direction))), 
                     directionalLight(color, intensity, direction, shadowInfo)
    {
        setupShader();
        setColor(color);
        lightShafts = PostProcessingRenderer::getLightShafts();
        if (lightShafts)
            lightShafts->setActive(true);
        setTexture(sunTexture);
        setActiveCameraDust(true);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Sun::~Sun()
    {
        lightShafts->setSunTexture(nullptr); // Back to default
    }

    //---------------------------------------------------------------------------
    //  Update
    //---------------------------------------------------------------------------

    void Sun::update(float delta)
    {
        Camera* camera = RenderingEngine::getCamera();

        const Point3f& cameraPosition = camera->getTransform().position;
        Transform& transform = getTransform();

        // Set the sun-mesh's position based on the current rotation and the camera's positon
        float zFar = camera->getZFar() * 0.75f;
        Vec3f facingDirection = transform.rotation.getForward();
        transform.position = cameraPosition - (facingDirection * zFar);

        // Sync transform of this node with the directional-light + lightshafts
        directionalLight.setRotation(transform.rotation);
        if (lightShafts)
        {
            lightShafts->setLightDirection(facingDirection);
            lightShafts->setScale(transform.scale);
        }

        Node::update(delta);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void Sun::setColor(const Color& color)
    {
        directionalLight.setColor(color);
        sunDiskShader->setColor("sunColor", color);
        if (lightShafts) lightShafts->setColor(color);
    }

    void Sun::setRedChannel(float val)
    {
        Color& color = directionalLight.getColor();
        color.r() = val;
        if(lightShafts) lightShafts->setColor(color);
        sunDiskShader->setColor("sunColor", color);
    }

    void Sun::setGreenChannel(float val)
    {
        Color& color = directionalLight.getColor();
        color.g() = val;
        if (lightShafts) lightShafts->setColor(color);
        sunDiskShader->setColor("sunColor", color);
    }

    void Sun::setBlueChannel(float val)
    {
        Color& color = directionalLight.getColor();
        color.b() = val;
        if (lightShafts) lightShafts->setColor(color);
        sunDiskShader->setColor("sunColor", color);
    }

    void Sun::setTexture(TexturePtr tex, bool lightShaftTexAswell)
    {
        sunDiskShader->setTexture("diffuse", tex);
        if(lightShaftTexAswell && lightShafts) lightShafts->setSunTexture(tex);
        getTransform().scale.x() *= tex->getAspecRatio();
    }

    void Sun::toggleCameraDust(){ SHADER("CameraDirt")->toggleActive(); }
    void Sun::setActiveCameraDust(bool b) { SHADER("CameraDirt")->setActive(b); }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    void Sun::setupShader()
    {
        sunTexture = TEXTURE("/textures/defaults/sun.dds");

        // same shader is used twice, but this here is used before tonemapping,
        // thats why i have to convert it to sRGB
        ForwardShaderParams params("Sun", "/shaders/post_process/sun", PipelineType::Sun);
        sunDiskShader = FORWARD_SHADER(params);
        sunDiskShader->setInt("toSRGB", 1);
        setMaterial(MATERIAL(sunDiskShader));
    }

    //---------------------------------------------------------------------------
    //  CSunComponent - Class
    //---------------------------------------------------------------------------

    //void Sun::CSunComp::update(float delta)
    //{
    //    Camera* currentCam = RenderingEngine::getCamera();
    //    Transform& transform = parentNode->getTransform();

    //    // Sun-Mesh is always facing to the camera
    //    transform.lookAt(currentCam->getTransform().position);

    //    // Set the sun-mesh's position based on the rotation of the directional-light
    //    transform.position = (Point3f)(-directionalLight->getDirection() * 800.0f);
    //}

}