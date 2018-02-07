#include "application.h"

#include "json scene/json_scene_manager.h"
#include "Input/input_manager.h"
#include "time/time_manager.h"


using namespace Pyro;

//---------------------------------------------------------------------------
//  Application class
//---------------------------------------------------------------------------

Application::Application()
    : window(512, 512), renderer(&window)
{
    this->startLoop();
}

Application::Application(int width, int height)
    : window(width, height), renderer(&window)
{
    this->startLoop();
}

class TestScene2 : public Scene
{
public:
    TestScene2() : Scene("TestScene2") {}
    ~TestScene2() {}

    void init(RenderingEngine* renderer) override
    {
        Camera* cam = new Camera(Transform(Point3f(0, 0, 20)));
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));
        renderer->setCamera(cam);

        Node* generalScriptNode = new Node("InteractionMaster");
        generalScriptNode->addComponent(new CInteract(70.0f));

        CubemapPtr cubemap = CUBEMAP({ "/textures/cubemaps/sunset.dds" });
        Skybox* skybox = new Skybox(cubemap);

        DirectionalLight* dirLight = new DirectionalLight(Color::WHITE, 2.0f, Vec3f(0, -1, -1), nullptr);

        auto crateMesh = MESH("/models/crate.obj");
        auto crateTex = TEXTURE("/textures/crate.dds");
        auto crateTexNormal = TEXTURE("/textures/crateNormal.dds");
        PBRMaterialPtr crateMat = PBRMATERIAL({ crateTex, crateTexNormal });

        //MeshPtr mesh = MESH("/models/streetlamp.obj");
        //Renderable* rend = new Renderable(mesh, crateMat, Transform(Point3f(0, 0, 0), Vec3f(1, 1, 1)));

        //GUIImage* shadowMap = new GUIImage(crateTex, Vec2f(), 400.0f, Color::WHITE, Anchor::BottomRight);
        //shadowMap->setLocalPos(Vec2f(-shadowMap->getWidth(), -shadowMap->getHeight()));
        //Node* fpsGUI = new Node("fpsGUI");
        //fpsGUI->addComponent(new GUI({ shadowMap }));

        float scale = 0.01f;
        Renderable* crate = new Renderable("Crate", crateMesh, crateMat, Transform(Point3f(0, 0, 0), Vec3f(scale,scale,scale)));
        crate->addComponent(new CRotate(1));
    }

};

class BloomTest : public JSONScene
{
public:
    BloomTest() : JSONScene(JSONSceneManager::loadFromFile("/scenes/bloom_test.json")) {}
    ~BloomTest() {
        if (SHADER_EXISTS("HDRBloom"))
            SHADER("HDRBloom")->setActive(false);
    }
};

class PBRTest : public Scene
{

public:
    PBRTest() : Scene("PBRTest") {}
    ~PBRTest() {}

    void init(RenderingEngine* renderer) override
    {
        Camera* cam = new Camera(Transform(Point3f(0, 0, 50)));
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));
        renderer->setCamera(cam);

        renderer->setUseIBL(true);
        renderer->setAmbientIntensity(1.0f);
        renderer->setEnvMapSpecularIntensity(1.0f);

        CubemapPtr sky0 = CUBEMAP("/textures/cubemaps/hill.dds");

        uint32_t irradianceSize = 128;
        IrradianceMapPtr irr0 = IRRADIANCEMAP(irradianceSize, sky0);

        uint32_t premSize = 512;
        PremPtr prem0 = PREM(premSize, sky0);

        renderer->setAmbientIrradianceMap(irr0);
        renderer->setEnvironmentMap(prem0);
        Skybox* skybox = new Skybox(sky0);

        DirectionalLight* dirLight = new DirectionalLight(Color::WHITE, 2.0f, Vec3f(0, -1, 1), nullptr);
        dirLight->addComponent(new CRotate(0.5f));

        float scale = 0.5f;
        Quatf rot = Quatf::rotationY(Mathf::deg2Rad(-90)) * Quatf::rotationX(Mathf::deg2Rad(-90));

        // PISTOL
        auto pistol_diffuse = TEXTURE("/textures/pbr/pistol/cerberus_a.jpg");
        auto pistol_normal = TEXTURE("/textures/pbr/pistol/cerberus_n.jpg");
        auto pistol_roughness = TEXTURE("/textures/pbr/pistol/cerberus_r.jpg");
        auto pistol_metallic = TEXTURE("/textures/pbr/pistol/cerberus_m.jpg");
        auto pistolMat = PBRMATERIAL({ pistol_diffuse, pistol_normal, pistol_roughness, pistol_metallic });

        auto pistolMesh = MESH("/models/cerberus.fbx");

        Renderable* pistol = new Renderable(pistolMesh, pistolMat, Transform(Point3f(25, 0, 0), Vec3f(scale, scale, scale), rot));

        // DAGGER
        auto dagger_diffuse = TEXTURE("/textures/pbr/dagger/dagger_a.tga");
        auto dagger_normal = TEXTURE("/textures/pbr/dagger/dagger_n.tga");
        auto dagger_roughness = TEXTURE("/textures/pbr/dagger/dagger_r.tga");
        auto dagger_metallic = TEXTURE("/textures/pbr/dagger/dagger_m.tga");
        auto daggerMat = PBRMATERIAL({ dagger_diffuse, dagger_normal, dagger_roughness, dagger_metallic });

        auto daggerMesh = MESH("/models/dagger.obj");

        //Quatf rot = Quatf::rotationY(Mathf::deg2Rad(-90)) * Quatf::rotationX(Mathf::deg2Rad(-90));
        Renderable* dagger = new Renderable(daggerMesh, daggerMat, Transform(Point3f(0, 20, 0), Vec3f(scale, scale, scale), rot));
    }

};

class PBRTest2 : public Scene
{

public:
    PBRTest2() : Scene("PBRTest2") {}
    ~PBRTest2() {}

    void init(RenderingEngine* renderer) override
    {
        Camera* cam = new Camera(Transform(Point3f(0, 0, 50)));
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));
        renderer->setCamera(cam);

        CubemapPtr sky0 = CUBEMAP("/textures/cubemaps/hill.dds");
        IrradianceMapPtr irr0 = IRRADIANCEMAP(128, sky0);
        PremPtr prem0 = PREM(128, sky0);

        renderer->setUseIBL(true);
        renderer->setAmbientIntensity(1.0f);
        renderer->setEnvMapSpecularIntensity(1.0f);

        renderer->setAmbientIrradianceMap(irr0);
        renderer->setEnvironmentMap(prem0);
        Skybox* skybox = new Skybox(sky0);

        DirectionalLight* dirLight = new DirectionalLight(Color::WHITE, 3.0f, Vec3f(0, -1, -1), nullptr);
        dirLight->addComponent(new CRotate(0.5f));

        auto tex0 = TEXTURE("/textures/defaults/white.dds");

        //Texture* grass_diffuse = new Texture("/textures/pbr/grass/albedo.png");
        //Texture* grass_normal = new Texture("/textures/pbr/grass/normal.png");
        //Texture* grass_roughness = new Texture("/textures/pbr/grass/roughness.png");
        //Texture* grass_metallic = nullptr;
        ////Texture* grass_height = new Texture("/textures/pbr/grass/height.png");
        //Texture* grass_ao = new Texture("/textures/pbr/grass/ao.png");
        //PBRMaterial* grassMat = new PBRMaterial(grass_diffuse, grass_normal, grass_roughness, grass_metallic);

        auto pbrMat = PBRMATERIAL({ tex0, 1.0f, 0.0f, Color::BLACK });

        float scale = 3.0f;
        //Sphere* gold = new Sphere(goldMat, Transform(Point3f(10, 0, 10), Vec3f(scale, scale, scale)));
        Sphere* sphere = new Sphere(pbrMat, Transform(Point3f(0, 0, 10), Vec3f(scale, scale, scale)));

        //Quad* plane = new Quad(grassMat, Transform(Point3f(0, -7, 0), Vec3f(100, 100, 1), Quatf::rotationX(Mathf::deg2Rad(90))));

        float rowMax = 6;
        for (unsigned i = 0; i < rowMax; i++)
        {
            float roughness = 0.05f + i / (rowMax - 1);

            for (unsigned j = 0; j < rowMax; j++)
            {
                float metallic = j / (rowMax - 1);
                float distance = 8.0f;
                float xOff = rowMax * distance * -0.5f;
                Point3f position(i * distance + xOff, j * distance, 0);

                auto mat = PBRMATERIAL({ tex0, roughness, metallic, Color::WHITE });
                Sphere* sphere = new Sphere(mat, Transform(position, Vec3f(scale, scale, scale)));
            }
        }

        //PointLight* pointLight = new PointLight(Color::WHITE, 300.0f, Vec3f(0, 0, 1), Point3f(-20, 20, 10), nullptr);
        //pointLight->addComponent(new CLightBillboard());
        //PointLight* pointLight2 = new PointLight(Color::WHITE, 300.0f, Vec3f(0, 0, 1), Point3f(20, 20, 10), nullptr);
        //pointLight2->addComponent(new CLightBillboard());
        //PointLight* pointLight3 = new PointLight(Color::WHITE, 300.0f, Vec3f(0, 0, 1), Point3f(-20, 10, 10), nullptr);
        //pointLight3->addComponent(new CLightBillboard());
        //PointLight* pointLight4 = new PointLight(Color::WHITE, 300.0f, Vec3f(0, 0, 1), Point3f(20, 10, 10), nullptr);
        //pointLight4->addComponent(new CLightBillboard());

        // DEBUG MENU
        DebugMenu* debugMenu = dynamic_cast<DebugMenu*>(SceneManager::getCurrentScene()->findNode("DebugMenu"));
        if (debugMenu == nullptr) return;
        debugMenu->addButton("Skybox Shader", nullptr);
        debugMenu->addButton("Basic", [=] { skybox->setCubemap(sky0); }, "Skybox Shader");
        debugMenu->addButton("Irradiance", [=] { skybox->setCubemap(irr0); }, "Skybox Shader");
        debugMenu->addButton("PREM", [=] { skybox->setCubemap(prem0); }, "Skybox Shader");
        debugMenu->addSliderButton("MipLevel", [=](float val) { skybox->setMipLevel(val * (prem0->numMips() - 1)); }, "Skybox Shader-PREM");

        debugMenu->addButton("Materials", nullptr);
        debugMenu->addButton("PbrMat", nullptr, "Materials");
        debugMenu->addSliderButton("Metallic", [=](float value) mutable { pbrMat->setMatMetallic(value); }, "Materials-PbrMat");
        debugMenu->addSliderButton("Roughness", [=](float value) mutable { pbrMat->setMatRoughness(value); }, "Materials-PbrMat");
        debugMenu->addButton("Color", nullptr, "Materials-PbrMat");
        debugMenu->addSliderButton("Red", [=](float value) mutable {pbrMat->setMatColor(Color(value, 0, 0)); }, "Materials-PbrMat-Color");
        debugMenu->addSliderButton("Green", [=](float value) mutable {pbrMat->setMatColor(Color(0, value, 0)); }, "Materials-PbrMat-Color");
        debugMenu->addSliderButton("Blue", [=](float value) mutable {pbrMat->setMatColor(Color(0, 0, value)); }, "Materials-PbrMat-Color");
        //debugMenu->addButton("Grass", nullptr, "Materials");
        //debugMenu->addButton("AOMap", [=] {  grassMat->setMatAOMap(grass_ao); }, "Materials-Grass");
        //debugMenu->addButton("NoneAOMap", [=] { grassMat->setMatAOMap(nullptr); }, "Materials-Grass");
        //debugMenu->addSliderButton("DispBias", [=](float value) { grassMat->setMatDisplacementBias(value * 0.2f - 0.1f); }, "Materials-Grass");
        //debugMenu->addSliderButton("DispScale", [=](float value) { grassMat->setMatDisplacementScale(value * 0.2f - 0.1f); }, "Materials-Grass");
    }

};

class PBRTest3 : public JSONScene
{
public:
    PBRTest3() : JSONScene(JSONSceneManager::loadFromFile("/scenes/pbr_materials.json")) {}
    ~PBRTest3() {}
};

class SponzaScene : public Scene
{
public:
    SponzaScene() : Scene("SponzaScene") {}
    ~SponzaScene() {}

    void init(RenderingEngine* renderer) override
    {
        Camera* cam = new Camera(Transform(Point3f(0, 0, 20)));
        cam->setZFar(3000);
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));
        cam->addComponent(new CLightSpawn(KeyCodes::X));
        renderer->setCamera(cam);

        Node* generalScriptNode = new Node("InteractionMaster");
        // Script gets disabled when debug-menu is open
        generalScriptNode->addComponent(new CInteract(70.0f));

        renderer->setUseIBL(false);
        CubemapPtr sky0 = CUBEMAP("/textures/cubemaps/tropical_sunny_day.dds");
        Skybox* skybox = new Skybox(sky0);

        DirectionalLight* dirLight = new DirectionalLight(Color::WHITE, 2.0f, Vec3f(0, -1, -1), nullptr);

        auto mesh = MESH("/models/sponza/sponza.obj");

        float scale = 0.1f;
        Renderable* sponza = new Renderable(mesh, Transform(Point3f(0, 0, 0), Vec3f(scale, scale, scale), Quatf::rotationY(Mathf::deg2Rad(0))));
    }

};

class TransformHierarchyScene : public Scene
{
public:
    TransformHierarchyScene() : Scene("TransformHierarchyScene") {}
    ~TransformHierarchyScene() {}

    void init(RenderingEngine* renderer) override
    {
        Camera* cam = new Camera(Transform(Point3f(0, 0, 20)));
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));
        cam->addComponent(new CInteract(70.0f));
        renderer->setCamera(cam);

        CubemapPtr cubemap = CUBEMAP("/textures/cubemaps/sunset.dds");
        Skybox* skybox = new Skybox(cubemap);

        float sphereScale = 3;
        Sphere* sphere2 = new Sphere("Basis", nullptr, Transform(Point3f(0, 0, 0), Vec3f(sphereScale, sphereScale, sphereScale)));
        sphere2->addComponent(new CRotate(1));
        Sphere* sphere3 = new Sphere("Inherited", nullptr, Transform(Point3f(6, 0, 0), Vec3f(1, 1, 1)));
        sphere2->addChild(sphere3);
        sphere3->addComponent(new CRotate(1));
        Sphere* sphere4 = new Sphere("Inherited2", nullptr, Transform(Point3f(3, 0, 0), Vec3f(1, 1, 1)));
        sphere3->addChild(sphere4);
    }
};

class TestScene : public Scene
{
public:
    TestScene() : Scene("TestScene") {}
    ~TestScene() {}

    void init(RenderingEngine* renderer) override
    {
        Camera* cam = new Camera(Transform(Point3f(0, 0, 20)));
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));
        cam->addComponent(new CLightSpawn(KeyCodes::B));
        //cam->addComponent(new CObjectSpawn(KeyCodes::B));

        renderer->setCamera(cam);

        renderer->setAmbientIntensity(0.15f);
        //renderer->setClearColor(Color::RED);
        //renderer->setResolutionMod(0.2f);
        //renderer->setRenderingMode(ERenderingMode::UNLIT);

        //---------------------------------------------------------------------------
        //  Lights
        //---------------------------------------------------------------------------

        //DirectionalLight* dirLight2 = new DirectionalLight(Color::WHITE, 2.0f, Vec3f(0, -1, -1), new ShadowInfo(11, 1.0f, 250));
        //dirLight->toggleActive();

        Sun* sun = new Sun(Vec3f(0.2, -0.4, 1), Vec2f(200, 200), Color::WHITE, 2.0f, new ShadowInfo(11, 1.0f, 250.0f));
        sun->addComponent(new CDefaultLensFlares(2.0f, 10.0f));

        uint32_t simulationTime = 300; // simulationTime seconds for 24h 
        TimeOfDay* timeOfDay = new TimeOfDay(renderer, simulationTime, sun);

        //sun->setActiveLightShafts(false);
        //sun->setActiveCameraDust(false);
        //sun->setTexture(new Texture("/textures/nyan.png"));

        PointLight* pointLight = new PointLight(Color::WHITE, 300.0f, Vec3f(0, 0, 1), Point3f(0, 10, 15), new ShadowInfo(10, 1.0f, VK_FILTER_LINEAR));
        pointLight->toggleActive();
        //pointLight->toggleShadows();
        //pointLight->addComponent(new CMove(Vec3f(10,0,0), 1));
        pointLight->addComponent(new CLightBillboard());
        //SpotLight* spotLight = new SpotLight(Color::WHITE, 1000, 90.0f, Vec3f(0, 0, 0.3f), Point3f(-8, -5, 0), Vec3f(1, 0, 0), new ShadowInfo(9));
        //spotLight->addComponent(new CLightBillboard());

        //---------------------------------------------------------------------------
        //  Meshes
        //---------------------------------------------------------------------------

        //Mesh* mesh = new Mesh("/models/sphere_uv.obj");

        //---------------------------------------------------------------------------
        //  Textures
        //---------------------------------------------------------------------------

        auto sampler = std::make_shared<Sampler>(1.0f, FILTER_LINEAR, FILTER_LINEAR, MIPMAP_MODE_LINEAR);
        auto samplerN = std::make_shared<Sampler>(1.0f, FILTER_NEAREST, FILTER_NEAREST, MIPMAP_MODE_NEAREST);

        auto tex_grass = TEXTURE({ "/textures/grass.dds", sampler });
        auto tex0 = TEXTURE({ "/textures/defaults/white.dds", sampler });
        auto tex1 = TEXTURE({ "/textures/defaults/black.dds", sampler });

        //---------------------------------------------------------------------------
        //  Materials
        //---------------------------------------------------------------------------
        auto grassMat = PBRMATERIAL({ tex_grass, 1.0f, 0.0f });
        grassMat->setMatUVScale(15.0f);

        CubemapPtr cubemap0 = CUBEMAP("/textures/cubemaps/tropical_sunny_day.dds");
        CubemapPtr cubemap1 = CUBEMAP("/textures/cubemaps/thick_clouds.dds");
        CubemapPtr cubemap2 = CUBEMAP("/textures/cubemaps/sunset.dds");

        uint32_t irradianceSize = 128;
        IrradianceMapPtr irr = IRRADIANCEMAP(irradianceSize, cubemap0);

        uint32_t premSize = 512;
        PremPtr prem = PREM(premSize, cubemap0);

        renderer->setUseIBL(true);
        renderer->setAmbientIrradianceMap(irr);
        renderer->setEnvironmentMap(prem);

        //---------------------------------------------------------------------------
        //  Objects
        //---------------------------------------------------------------------------
#define SKYBOX 1
#if SKYBOX
        //Cubemap* plTex = dynamic_cast<Cubemap*>(pointLight->getShadowMapTex());
        Skybox* skybox = timeOfDay->getSkybox();
#endif
        Quad* plane = new Quad(grassMat, Transform(Point3f(0, -7, 0), Vec3f(100, 100, 1), Quatf::rotationX(Mathf::deg2Rad(90))));

        float sphereScale = 3.0f;
        float rowMax = 6;
        for (unsigned i = 0; i < rowMax; i++)
        {
            float roughness = 0.05f + i / (rowMax - 1);

            for (unsigned j = 0; j < rowMax; j++)
            {
                float metallic = j / (rowMax - 1);
                float distance = 8.0f;
                float xOff = rowMax * distance * -0.5f;
                Point3f position(i * distance + xOff, j * distance, 0);

                auto mat = PBRMATERIAL({ tex0, roughness, metallic, Color::WHITE });
                Sphere* sphere = new Sphere(mat, Transform(position, Vec3f(sphereScale, sphereScale, sphereScale)));
            }
        }
        
        //---------------------------------------------------------------------------
        //  Other
        //---------------------------------------------------------------------------

        // Name is important, because the debug-menu searches this node by name to disable the CInteract-script when active
        Node* generalScriptNode = new Node("InteractionMaster");
        // Script gets disabled when debug-menu is open
        generalScriptNode->addComponent(new CInteract(70.0f));

        attachInputFunc(KeyCodes::N, [=] {
            static int i = 0;
            std::string filePath = "test_#" + std::to_string(i++) + ".png";
            renderer->setRenderCallback([=](const ImageData& imageData) {
                // NEVER USE BY REFERENCE "[&]" for a lambda in a lambda 
                WRITE_IMAGE(filePath, imageData);
            });
        }, Input::KEY_PRESSED);

#ifdef FREETYPE_LIB
        // Depth-Texture shown in the HUD (TOGGLE with "M")
        GUIImage* shadowMap = new GUIImage(sun->getDirLight()->getShadowMapTex(), Vec2f(), 400.0f, Color::WHITE, Anchor::BottomRight);
        shadowMap->setActive(false);
        shadowMap->setLocalPos(Vec2f(-shadowMap->getWidth(), -shadowMap->getHeight()));
        attachInputFunc(KeyCodes::M, [=] { shadowMap->toggleActive(); }, Input::KEY_PRESSED);

        Node* guiNode = new Node("GUINode");
        guiNode->addComponent(new GUI({ shadowMap }));

        // DEBUG MENU
        DebugMenu* debugMenu = dynamic_cast<DebugMenu*>(SceneManager::getCurrentScene()->findNode("DebugMenu"));
        if(debugMenu == nullptr) return;
        {
            debugMenu->addButton("Lights", nullptr);  
            debugMenu->addButton("Point-Light", nullptr, "Lights");
            debugMenu->addButton("Toggle Active", [=] { pointLight->toggleActive(); }, "Lights-Point-Light");
            debugMenu->addButton("Toggle Static/Dynamic", [=] { pointLight->toggleType(); }, "Lights-Point-Light");
            debugMenu->addSliderButton("Intensity", [=](float value) { pointLight->setIntensity(value * 500); }, "Lights-Point-Light");
            debugMenu->addButton("Attenuation", nullptr, "Lights-Point-Light");
            debugMenu->addSliderButton("Constant", [=](float value) {pointLight->setConstant(value * 5); }, "Lights-Point-Light-Attenuation");
            debugMenu->addSliderButton("Linear", [=](float value) {pointLight->setLinear(value * 5); }, "Lights-Point-Light-Attenuation");
            debugMenu->addSliderButton("Exponent", [=](float value) {pointLight->setExponent(value * 5); }, "Lights-Point-Light-Attenuation");
            debugMenu->addButton("Color", nullptr, "Lights-Point-Light");
            debugMenu->addSliderButton("Red", [=](float value) {pointLight->getColor().r() = value; }, "Lights-Point-Light-Color");
            debugMenu->addSliderButton("Green", [=](float value) {pointLight->getColor().g() = value; }, "Lights-Point-Light-Color");
            debugMenu->addSliderButton("Blue", [=](float value) {pointLight->getColor().b() = value; }, "Lights-Point-Light-Color");
            debugMenu->addButton("Position", nullptr, "Lights-Point-Light");
            debugMenu->addSliderButton("X", [=](float value) {pointLight->getTransform().position.x() = value * 100 - 50; }, "Lights-Point-Light-Position");
            debugMenu->addSliderButton("Y", [=](float value) {pointLight->getTransform().position.y() = value * 100 - 50; }, "Lights-Point-Light-Position");
            debugMenu->addSliderButton("Z", [=](float value) {pointLight->getTransform().position.z() = value * 100 - 50; }, "Lights-Point-Light-Position");

            debugMenu->addButton("TimeOfDay", nullptr);

            debugMenu->addButton("Sun", nullptr, "TimeOfDay");
            if (SHADER_EXISTS("LightShafts"))
            {
                debugMenu->addButton("LightShafts", nullptr, "TimeOfDay-Sun");
                debugMenu->addButton("Toggle", [=] { sun->toggleLightShafts(); }, "TimeOfDay-Sun-LightShafts");
                debugMenu->addSliderButton("Scale", [=](float value) { SHADER("LightShafts#RadialBlur")->setFloat("radialBlurScale", value); }, "TimeOfDay-Sun-LightShafts");
                debugMenu->addSliderButton("Strength", [=](float value) { SHADER("LightShafts#RadialBlur")->setFloat("radialBlurStrength", value * 2); }, "TimeOfDay-Sun-LightShafts");
            }
            debugMenu->addButton("Shadows", nullptr, "TimeOfDay-Sun");
            debugMenu->addButton("Toggle", [=] { sun->getDirLight()->toggleShadows(); }, "TimeOfDay-Sun-Shadows");
            debugMenu->addSliderButton("Shadow-Softness", [=](float value) { sun->getDirLight()->setShadowBlurScale(value * 2); }, "TimeOfDay-Sun-Shadows");
            debugMenu->addSliderButton("MinVariance", [=](float value) { sun->getDirLight()->setMinVariance(value * 0.00001f); }, "TimeOfDay-Sun-Shadows");
            debugMenu->addSliderButton("LinStep", [=](float value) { sun->getDirLight()->setLinStep(value); }, "TimeOfDay-Sun-Shadows");
            debugMenu->addSliderButton("ShadowDistance", [=](float value) { sun->getDirLight()->setShadowDistance(value * 1000); }, "TimeOfDay-Sun-Shadows");

            debugMenu->addSliderButton("Disk-Scale", [=](float value) { sun->getTransform().setScale(value * 1000.0f); }, "TimeOfDay-Sun");

            debugMenu->addButton("Toggle Type", [=] { sun->getDirLight()->toggleType(); }, "TimeOfDay-Sun");
            debugMenu->addSliderButton("Intensity", [=](float value) { sun->getDirLight()->setIntensity(10 * value); }, "TimeOfDay-Sun");

            debugMenu->addButton("Rotation", nullptr, "TimeOfDay-Sun");
            debugMenu->addSliderButton("Yaw", [=](float value) { sun->getTransform().setYRotation(value * 360); }, "TimeOfDay-Sun-Rotation");
            debugMenu->addSliderButton("Pitch", [=](float value) { sun->getTransform().setXRotation(value * 360); }, "TimeOfDay-Sun-Rotation");

            debugMenu->addButton("Color", nullptr, "TimeOfDay-Sun");
            debugMenu->addSliderButton("Red", [=](float value) { sun->setRedChannel(value); }, "TimeOfDay-Sun-Color");
            debugMenu->addSliderButton("Green", [=](float value) { sun->setGreenChannel(value); }, "TimeOfDay-Sun-Color");
            debugMenu->addSliderButton("Blue", [=](float value) { sun->setBlueChannel(value); }, "TimeOfDay-Sun-Color");

            debugMenu->addButton("Toggle Simulation", [=] { timeOfDay->toggleSimulation(); }, "TimeOfDay");
            debugMenu->addSliderButton("Simulation Speed", [=](float value) { timeOfDay->setSimulationSpeed(value * 10); }, "TimeOfDay");
            debugMenu->addSliderButton("DayTime", [=](float value) { timeOfDay->setDayTimeAsPercentage(value); }, "TimeOfDay");


            //---------------------------------------------------------------------------
            //  Materials
            //---------------------------------------------------------------------------

            debugMenu->addButton("Materials", nullptr);
            //debugMenu->addButton("PbrMat", nullptr, "Materials");
            //debugMenu->addSliderButton("Metallic", [=](float value) { pbrMat->setMatMetallic(value); }, "Materials-PbrMat");
            //debugMenu->addSliderButton("Roughness", [=](float value) { pbrMat->setMatRoughness(value); }, "Materials-PbrMat");
            //debugMenu->addButton("Crate", nullptr, "Materials");
            //debugMenu->addSliderButton("Metallic", [&](float value) { crateMat.setMatMetallic(value); }, "Materials-Crate");
            //debugMenu->addSliderButton("Roughness", [&](float value) { crateMat.setMatRoughness(value); }, "Materials-Crate");
            debugMenu->addButton("Plane", nullptr, "Materials");
            debugMenu->addSliderButton("Texture-Scale", [=](float value) mutable { grassMat->setMatUVScale(value * 20); }, "Materials-Plane");
            debugMenu->addSliderButton("Metallic", [=](float value) mutable { grassMat->setMatMetallic(value); }, "Materials-Plane");
            debugMenu->addSliderButton("Roughness", [=](float value) mutable { grassMat->setMatRoughness(value); }, "Materials-Plane");

            //---------------------------------------------------------------------------
            //  Objects
            //---------------------------------------------------------------------------

            //debugMenu->addButton("Object", nullptr);
            //debugMenu->addButton("Toggle Active", [=] { node.toggleActive(); }, "Object");
            //debugMenu->addButton("Transform", nullptr, "Object");
            //debugMenu->addSliderButton("Scale", [=](float value) { node.getTransform().scale = Vec3f(value * 20, value * 20, value * 20); }, "Object-Transform");
            //debugMenu->addButton("Rotation", nullptr, "Object-Transform");
            //debugMenu->addSliderButton("X", [=](float value) {node.getTransform().rotation = Quatf::euler(value * 360, 0, 0); }, "Object-Transform-Rotation");
            //debugMenu->addSliderButton("Y", [=](float value) {node.getTransform().rotation = Quatf::euler(0, value * 360, 0); }, "Object-Transform-Rotation");
            //debugMenu->addSliderButton("Z", [=](float value) {node.getTransform().rotation = Quatf::euler(0, 0, value * 360); }, "Object-Transform-Rotation");

#if SKYBOX
            // Skybox loading
            {
                debugMenu->addButton("Skybox", nullptr);
                debugMenu->addButton("SecondCubemap", nullptr, "Skybox");
                debugMenu->addSliderButton("Blend-Factor", [=](float value) { skybox->setBlendFactor(value); }, "Skybox-SecondCubemap");

                debugMenu->addButton("Cubemap 0", [=] {
                    skybox->setSecondCubemap(cubemap0);
                }, "Skybox-SecondCubemap");

                debugMenu->addButton("Cubemap 1", [=] {
                    skybox->setSecondCubemap(cubemap1);
                }, "Skybox-SecondCubemap");
                debugMenu->addButton("Cubemap 2", [=] {
                    skybox->setSecondCubemap(cubemap2);
                }, "Skybox-SecondCubemap");

                debugMenu->addButton("Cubemap 0", [=] {
                    skybox->setCubemap(cubemap0);
                }, "Skybox");

                debugMenu->addButton("Cubemap 1", [=] {
                    skybox->setCubemap(cubemap1);
                }, "Skybox");
                debugMenu->addButton("Cubemap 2", [=] {
                    skybox->setCubemap(cubemap2);
                }, "Skybox");

#if SKYBOX && 0
                debugMenu->addSliderButton("Skybox-Upper-Limit", [=](float value) { skybox->setFogUpperLimit(value * 10); }, "Skybox");
                debugMenu->addSliderButton("Skybox-Lower-Limit", [=](float value) { skybox->setFogLowerLimit(value * -10); }, "Skybox");
#endif
            }
#endif
        }
#endif

    }

};

class MyScene : public Scene
{
    RenderingEngine*    renderer;
    Camera*             cam;
    VkFormat            format; // The format of the image on the gpu
    uint32_t            width;
    uint32_t            height;
    uint32_t            bytesPerPixel = 4;
    unsigned char       col = 0;
    unsigned char*      data;

public:
    MyScene() : Scene("MyScene") {}
    ~MyScene() { delete data; }

    // Called once to initialize everything for this scene
    void init(RenderingEngine* _renderer) override
    {
        renderer = _renderer;

        // NOTICE: All ENGINE-objects created with "new" are managed by the engine and will be deleted automatically
        cam = new Camera(Transform(Point3f(0, 3, 20)), Camera::EMode::PERSPECTIVE);

        // Use the "CUSTOM" camera mode to manually set the view- & projection- matrix via "setViewMatrix(..)" + "setProjectionMatrix(..)"
        // Scroll further down to the render-loop to see how it works exactly.
        //Camera* cam = new Camera(Transform(Point3f(0, 3, 20)), Camera::EMode::CUSTOM);
        cam->setZFar(3000.0f);
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));

        renderer->setCamera(cam);

        // Important stuff starts here

        // This information should be from your video-stream
        format = VK_FORMAT_B8G8R8A8_UNORM; // The format of the image on the gpu
        width = Window::getWidth();
        height = Window::getHeight();

        // Tell the renderer to enable the PreProcess-Pipeline. It allocates the image on the GPU, 
        // which is rendered as a fullscreen quad BEFORE the 3d-scene
        renderer->createPreProcessBuffer(Vec2ui(width, height), format);

        // Test-Data. This could be YOUR video stream. Note that the data in it has to match the given format above.
        // Scroll further down to the update method to see how i fill this test-data.
        uint32_t size = width * height * bytesPerPixel;
        data = new unsigned char[size];

        //---------------------------------------------------------------------------
        //  Lights
        //---------------------------------------------------------------------------

        DirectionalLight* dirLight = new DirectionalLight(Color::WHITE, 2.0f, Vec3f(0, -1, -1), new ShadowInfo(11, 1.0f, 100));
        //PointLight pointLight(Color::WHITE, 300, Vec3f(0, 0, 0.5f), Point3f(0, 20, 15), new ShadowInfo(10, 1.0f, VK_FILTER_LINEAR));
        //SpotLight spotLight(Color::WHITE, 100, 90.0f, Vec3f(0.0f, 0.0f, 0.3f), Point3f(-8, -5, 0), Vec3f(1, 0, 0), new ShadowInfo(9));

        //---------------------------------------------------------------------------
        //  Meshes
        //---------------------------------------------------------------------------

        //Mesh* sponzaMesh = new Mesh("/models/sponza/sponza.obj");
        //Mesh* mesh = new Mesh("/models/sphere_uv.obj");

        //---------------------------------------------------------------------------
        //  Textures
        //---------------------------------------------------------------------------
        std::shared_ptr<Sampler> sampler(new Sampler(8.0f, FILTER_LINEAR, FILTER_LINEAR, MIPMAP_MODE_LINEAR));

        auto tex_grass = TEXTURE({ "/textures/grass.dds", sampler });

        //---------------------------------------------------------------------------
        //  Shaders
        //---------------------------------------------------------------------------

        // If you need or want to create a custom shader ;)
        //ForwardShader* reflect = new ForwardShader("Reflect", "/shaders/reflect", PipelineType::Basic);

        //---------------------------------------------------------------------------
        //  Materials
        //---------------------------------------------------------------------------

        // Constructor: Texture, Roughness, Metallness
        auto grassMat = PBRMATERIAL({ tex_grass, 1.0f, 0.0f });
        grassMat->setMatUVScale(15.0f);

        //---------------------------------------------------------------------------
        //  Objects
        //---------------------------------------------------------------------------

        Quad* plane = new Quad(grassMat, Transform(Point3f(0, -3, 0), Vec3f(100, 100, 1), Quatf::rotationX(Mathf::deg2Rad(90))));

        Cube* cube = new Cube(nullptr, Transform());

        // This is how to define a object with a separate mesh & material
        //Renderable* crate = new Renderable("Crate", mesh, mat, Transform(Point3f(0, 0, 10), Vec3f(0.01f, 0.01f, 0.01f)));
    }

    // Called every frame
    void update(float delta) override
    {
        // If CUSTOM camera-mode is enabled use this functions to set the view & projection
        //Mat4f view = Mat4f::view(position, forwardVec, upWardVec);
        //cam->setViewMatrix(view);

        //Mat4f projection = Mat4f::perspective(Mathf::deg2Rad(fov), ar, zNear, zFar);
        //cam->setProjectionMatrix(projection);

        // DO PER FRAME STUFF HERE
        col = (col + 1) % 255; // modify test-data
        for (unsigned int i = 0; i < height; i++)
        {
            unsigned char* row = data + i * width * bytesPerPixel;
            for (unsigned int j = 0; j < width; j++)
            {
                unsigned char* pixel = row + j * bytesPerPixel;
                pixel[0] = col;
                pixel[1] = 0;
                pixel[2] = 0;
                pixel[3] = 255;
            }
        }
        // Push the data onto the GPU.
        renderer->fillPreprocessBuffer(data);
    }

};

class ResourceTestScene : public Scene
{
public:
    ResourceTestScene() : Scene("ResourceTestScene") {}
    ~ResourceTestScene() {}
    
    void init(RenderingEngine* renderer) override
    {
        Camera* cam = new Camera(Transform(Point3f(0, 0, 20)));
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));
        renderer->setCamera(cam);
        cam->addComponent(new CInteract(70.0f));
        cam->addComponent(new CObjectSpawn(KeyCodes::X));

        std::string fp = "/models/test.obj";
        auto m = MESH(fp);

        auto tex = TEXTURE("/textures/rock.jpg");
        PBRMaterialPtr mat = PBRMATERIAL(tex);

        Renderable* r = new Renderable(m, mat, Transform(Point3f(0, 0, 0), Vec3f(3, 3, 3)));

        attachInputFunc(KeyCodes::U, [=] { 
            Logger::Log("U PRESSED");
            /* Change mesh */
            auto barrel = MESH("/models/barrel.obj");
            r->setMesh(barrel);
        }, Input::KEY_PRESSED);

        //setInterval([&]{ 
        //    Logger::Log("Num Meshes: " + TS(NUM_MESHES));
        //}, 5000);
    }
};

class Schatzsuche : public Scene
{
    Renderable* crate;
    Sphere* vertices[5];

public:
    Schatzsuche() : Scene("Schatzsuche") {}
    ~Schatzsuche() {}

    int numSpheres = 0;

    void init(RenderingEngine* renderer) override
    {
        Camera* cam = new Camera(Transform(Point3f(0, 0, 20)));
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));
        renderer->setCamera(cam);

        CubemapPtr cubemap = CUBEMAP({ "/textures/cubemaps/sunset.dds" });
        Skybox* skybox = new Skybox(cubemap);

        DirectionalLight* dirLight = new DirectionalLight(Color::WHITE, 2.0f, Vec3f(0, -1, -1), nullptr);

        auto crateMesh = MESH("/models/crate.obj");
        auto crateTex = TEXTURE("/textures/crate.dds");
        auto crateTexNormal = TEXTURE("/textures/crateNormal.dds");
        PBRMaterialPtr crateMat = PBRMATERIAL({ crateTex, crateTexNormal });

        PBRMaterialPtr sphereMat = PBRMATERIAL({ crateTex });
        sphereMat->setMatRoughness(0.0f);
        sphereMat->setMatMetallic(0.0f);

        Vec3f scale(0.2f, .2f, .2f);
        vertices[0] = new Sphere(sphereMat, Transform(Point3f(-7, -7, 0), scale));
        vertices[1] = new Sphere(sphereMat, Transform(Point3f(7, -7, 0), scale));
        vertices[2] = new Sphere(sphereMat, Transform(Point3f(0, 7, 0), scale));
        vertices[3] = new Sphere(sphereMat, Transform(Point3f(0, -7, -7), scale));
        vertices[4] = new Sphere(sphereMat, Transform(Point3f(0, -7, 7), scale));

        crate = new Renderable("Crate", crateMesh, crateMat, Transform(Point3f(0, 0, 0), Vec3f(0.005f, 0.005f, 0.005f)));
        crate->addComponent(new CRotate(1));

    }

    void update(float delta) override
    {
            numSpheres++;
            if (numSpheres > 2000)
                return;

            int randVertex = Random::randomInt(4);

            Vec3f dirToVertex = vertices[randVertex]->getTransform().position - crate->getTransform().position;

            crate->getTransform().position += dirToVertex * 0.5f;

            new Sphere(Transform(crate->getTransform().position, Vec3f(.3f, .3f, .3f)));
    }
};

class Fabriken : public Scene
{
    Renderable* crate;
    Sphere* vertices[5];

public:
    Fabriken() : Scene("Fabriken") {}
    ~Fabriken() {}

#define SIZE 50
    Renderable* fabriken[SIZE];
    bool hasFabrik[SIZE];

    void init(RenderingEngine* renderer) override
    {
        Camera* cam = new Camera(Transform(Point3f(0, 0, 20)));
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));
        renderer->setCamera(cam);

        CubemapPtr cubemap = CUBEMAP({ "/textures/cubemaps/sunset.dds" });
        Skybox* skybox = new Skybox(cubemap);

        DirectionalLight* dirLight = new DirectionalLight(Color::WHITE, 2.0f, Vec3f(0, -1, -1), nullptr);

        auto mesh = MESH("/models/crate.obj");
        auto crateTex = TEXTURE("/textures/crate.dds");
        auto crateTexNormal = TEXTURE("/textures/crateNormal.dds");
        PBRMaterialPtr crateMat = PBRMATERIAL({ crateTex, crateTexNormal });

        for (int i = 0; i < SIZE; i++)
        {
            float xPos = -30 + i * 1.5f;
            bool create = Random::randomInt();
            if (create)
            {
                hasFabrik[i] = true;
                fabriken[i] = new Renderable(mesh, crateMat, Transform(Point3f(xPos, 0, 0), Vec3f(0.005f, 0.005f, 0.005f)));
            }
            else
            {
                hasFabrik[i] = false;
                fabriken[i] = nullptr;
            }
        }

        Time::setInterval([=] {

            for (int i = 0; i < SIZE; i++)
            {
                bool leftleft   = i < 2 ? false : hasFabrik[i-2];
                bool left       = i < 1 ? false : hasFabrik[i-1];
                bool center     = hasFabrik[i];
                bool right      = i > (SIZE - 2) ? false : hasFabrik[i+1];
                bool rightright = i > (SIZE - 3) ? false : hasFabrik[i+2];

                int numNeighbors = leftleft + left + center + right + rightright;

                if (numNeighbors == 2 || numNeighbors == 4)
                {
                    if (fabriken[i] == nullptr)
                    {
                        float xPos = -30 + i * 1.5f;
                        fabriken[i] = new Renderable(mesh, crateMat, Transform(Point3f(xPos, 0, 0), Vec3f(0.005f, 0.005f, 0.005f)));
                    }
                }
                else
                {
                    if (fabriken[i] != nullptr)
                    {
                        delete fabriken[i];
                        fabriken[i] = nullptr;
                    }
                }
            }

            for (int i = 0; i < SIZE; i++)
            {
                hasFabrik[i] = fabriken[i] != nullptr;
            }

        }, 1000);
    }
};

class AudiA6Scene : public Scene
{
public:
    AudiA6Scene() : Scene("AudiA6") {}
    ~AudiA6Scene() {}

    void init(RenderingEngine* renderer) override
    {
        Camera* cam = new Camera(Transform(Point3f(0, 0, 3)));
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));
        //cam->addComponent(new CInteract(70.0f));
        renderer->setCamera(cam);
        renderer->setAmbientIntensity(1.0f);

        CubemapPtr cubemap = CUBEMAP("/textures/cubemaps/hill.dds");
        Skybox* skybox = new Skybox(cubemap);

        IrradianceMapPtr irr = IRRADIANCEMAP(128, cubemap);
        PremPtr prem = PREM(256, cubemap);
        renderer->setEnvironmentMap(prem);
        renderer->setAmbientIrradianceMap(irr);

        MeshPtr carMesh = MESH({ "A6/a6.obj" });

        float scale = 1.0f;
        Transform trans(Point3f(-2.4f, -0.7f, 0), Vec3f(scale, scale, scale));
        Renderable* car = new Renderable(carMesh, trans);

        // DEBUG MENU
        DebugMenu* debugMenu = dynamic_cast<DebugMenu*>(SceneManager::getCurrentScene()->findNode("DebugMenu"));
        debugMenu->addButton("Materials", nullptr);

        for (unsigned int i = 0; i < carMesh->numMaterials(); i++)
        {
            auto mat = carMesh->getMaterial(i);
            auto pbrMat = (PBRMaterialPtr) mat;
            auto name = mat->getName();

            if (name == "Lack")
            {
                pbrMat->setMatRoughness(0.05f);
                pbrMat->setMatMetallic(0.4f);
            }

            debugMenu->addButton(name, nullptr, "Materials");
            std::string dirName = "Materials-" + name;
            debugMenu->addSliderButton("Metallic", [=](float value) mutable { pbrMat->setMatMetallic(value); }, dirName);
            debugMenu->addSliderButton("Roughness", [=](float value) mutable { pbrMat->setMatRoughness(value); }, dirName);
            debugMenu->addButton("Color", nullptr, dirName);
            debugMenu->addSliderButton("Red", [=](float value) mutable { Color c = pbrMat->getMatColor(); c.r() = value; pbrMat->setMatColor(c); }, dirName + "-Color");
            debugMenu->addSliderButton("Green", [=](float value) mutable { Color c = pbrMat->getMatColor(); c.g() = value; pbrMat->setMatColor(c);  }, dirName + "-Color");
            debugMenu->addSliderButton("Blue", [=](float value) mutable { Color c = pbrMat->getMatColor(); c.b() = value; pbrMat->setMatColor(c); }, dirName + "-Color");
        }
    }
};

//std::string sceneJSON = "/scenes/scene0.json";
std::string sceneJSON = "scene.json";
std::string jsonFile2 = "/scenes/scene1.json";
std::string jsonFile3 = "/scenes/test_scene_2.json";

void Application::startLoop()
{
    renderer.setVSync(true);
    Input::attachFunc(KeyCodes::C, [&] {renderer.toggleCulling(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::G, [&] {renderer.toggleGUI(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::V, [&] {renderer.toggleVSync(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::Z, [&] {renderer.toggleShadows(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::P, [&] {renderer.togglePostProcessing(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::F, [&] {renderer.toggleBoundingBoxes(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::H, [&] { SHADER("FXAA")->toggleActive(); }, Input::KEY_PRESSED);

    Input::attachFunc(KeyCodes::THREE, [&] { JSONSceneManager::switchSceneFromFile(sceneJSON); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::FOUR, [&] { JSONSceneManager::switchSceneFromFile(jsonFile2); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::FIVE, [&] { JSONSceneManager::switchSceneFromFile(jsonFile3); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::SIX,   [&] { SceneManager::switchScene(new BloomTest()); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::SEVEN, [&] { SceneManager::switchScene(new TransformHierarchyScene()); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::EIGHT, [&] { SceneManager::switchScene(new SponzaScene()); }, Input::KEY_PRESSED);

    // Change Window-Title text every second (1000ms)
    Time::setInterval([&] { 
        //std::cout << "DELTA: " << Time::getDelta() / (float)Time::MILLISECOND << " ms" << std::endl;
        std::string windowTitle = "FPS: " + std::to_string(Time::getFPS()) + " (" + std::to_string(1000.0f / Time::getFPS()) + " ms)";
        window.setWindowText(windowTitle.c_str()); }
    , 1000);

    initDebugMenu();

    //JSONSceneManager::switchSceneFromFile("/scenes/dagger_pistol_pbr.json"));
    //JSONSceneManager::switchSceneFromFile("/scenes/test_scene_2.json"));
    //JSONSceneManager::switchSceneFromFile(sceneJSON);
    SceneManager::switchScene(new TestScene());

    //JSONSceneManager::setCleanupStrategy(ECleanupStrategy::TIMER, 10.0f);
    //JSONSceneManager::setCleanupStrategy(ECleanupStrategy::FIXED_AMOUNT_OF_SCENES, 2);
    Logger::setLogLevel(LOG_LEVEL_IMPORTANT);
    JSONSceneManager::setHotReloading(true, 0.5f);
    JSONSceneManager::setCleanupStrategy(ECleanupStrategy::FULL_UTILIZATION);

    // Render-Loop
    while (window.update() && !Input::getKeyDown(KeyCodes::ESCAPE))
    {
        TimeManager::update();
        float delta = static_cast<float>(Time::getDeltaSeconds());
        InputManager::update(delta);

        renderer.update(delta);
        renderer.draw();
    }
}


void Application::initDebugMenu()
{
    DebugMenu* d = new DebugMenu(&renderer, FONT_GET(DEBUG_FONT, 24));
    d->addButton("Scenes", nullptr);
    d->addButton("Empty Scene", []() { SceneManager::switchScene(new EmptyScene()); }, "Scenes");
    d->addButton("JSON Scene", []() { JSONSceneManager::switchSceneFromFile(sceneJSON); }, "Scenes");
    d->addButton("Test Scene", []() { SceneManager::switchScene(new TestScene()); }, "Scenes");
    d->addButton("Cube-Scene", []() { SceneManager::switchScene(new TestScene2()); }, "Scenes");
    d->addButton("Bloom-Test", []() { SceneManager::switchScene(new BloomTest()); }, "Scenes");
    d->addButton("PBR-Test", []() { SceneManager::switchScene(new PBRTest()); }, "Scenes");
    d->addButton("PBR-Test2", []() { SceneManager::switchScene(new PBRTest2()); }, "Scenes");
    d->addButton("PBR-Test3", []() { SceneManager::switchScene(new PBRTest3()); }, "Scenes");
    d->addButton("Transform-Hierarchy", []() { SceneManager::switchScene(new TransformHierarchyScene()); }, "Scenes");
    d->addButton("Sponza", []() { SceneManager::switchScene(new SponzaScene()); }, "Scenes");

    std::vector<std::string> cubemaps = { 
        "/textures/cubemaps/hill.dds",
        "/textures/cubemaps/flat.dds",
        "/textures/cubemaps/green.dds",
        "/textures/cubemaps/full_moon.dds",
        "/textures/cubemaps/sunset.dds",
        "/textures/cubemaps/thick_clouds.dds",
        "/textures/cubemaps/cloudy_light_rays.dds",
        "/textures/cubemaps/dark_stormy.dds",
        "/textures/cubemaps/tropical_sunny_day.dds" };

    d->addButton("Next EnvMap", [=] {
        static int i = 0;
        CubemapPtr sky = CUBEMAP(cubemaps[i]);
        i = (i + 1) % cubemaps.size();
        IrradianceMapPtr irr = IRRADIANCEMAP(128, sky);
        PremPtr prem = PREM(256,sky);
        Skybox* skybox = dynamic_cast<Skybox*>( SceneManager::getCurrentScene()->findRenderable("Skybox"));
        if(skybox)skybox->setCubemap(sky);
        renderer.setAmbientIrradianceMap(irr);
        renderer.setEnvironmentMap(prem);
    });

    d->save();
}