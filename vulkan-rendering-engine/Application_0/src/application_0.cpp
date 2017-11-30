#include "application.h"

#include "threading/thread_pool.hpp"
#include "Input/input_manager.h"

// This Applications runs with the standard blin-phong pipeline.
// CURRENTLY BROKEN BECAUSE THE ENGINE DOES SUPPORT ONLY PBR NOW
// I MAY REMOVE THIS LATER

//---------------------------------------------------------------------------
//  Application class
//---------------------------------------------------------------------------

Application::Application() 
    : window(512, 512), renderer(&window)
{
    this->run();
}

Application::Application(int width, int height)
    : window(width, height), renderer(&window)
{
    this->run();
}


void Application::run()
{
    Input::attachFunc(KeyCodes::C, [&] {renderer.toggleCulling(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::G, [&] {renderer.toggleGUI(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::V, [&] {renderer.toggleVSync(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::Z, [&] {renderer.toggleShadows(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::P, [&] {renderer.togglePostProcessing(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::H, [&] { Shader::get("FXAA")->toggleActive(); }, Input::KEY_PRESSED);

    Camera cam(Transform(Point3f(0,3,20)));
    cam.setZFar(3000.0f);
    cam.addComponent(new CMoveCamera(70, 3, 5));
    //cam.addComponent(new CLightSpawn(KeyCodes::N)); //TODO: RUNTIME SPAWN-MANAGER
    //SpotLight cameraLight(Color::WHITE, 100, 90.0f);
    //cam.addChild(&cameraLight);

    renderer.setCamera(&cam);
    renderer.setAmbientIntensity(0.4f);

    // Spawn Lights
    std::vector<std::unique_ptr<PointLight>> pointLights;
    {
        Input::attachFunc(KeyCodes::N, [&] {
            Color randomColor(Random::randomFloat(), Random::randomFloat(), Random::randomFloat());
            Point3f spawnPosition = cam.getWorldPosition();
            float intensity = 20.0f;
            Vec3f attenuation = Vec3f(.0f, .0f, 0.5f);
            std::unique_ptr<PointLight> pointLight(new PointLight(randomColor, intensity, attenuation, spawnPosition, nullptr));
            pointLight->addComponent(new CMove(Vec3f(40, 0, 0), 1.0f));
            pointLights.push_back(std::move(pointLight));
        }, Input::KEY_PRESSED);
    }

    // Name is important, because the debug-menu searches this node by name to disable the CInteract-script when active
    Node generalScriptNode("InteractionMaster");
    // Script gets disabled when debug-menu is open
    generalScriptNode.addComponent(new CInteract(70.0f));

    std::unique_ptr<Cubemap> cubemap0 = std::unique_ptr<Cubemap>(new Cubemap("/textures/cubemaps/tropical_sunny_day.ktx"));
    std::unique_ptr<Cubemap> cubemap1 = std::unique_ptr<Cubemap>(new Cubemap("/textures/cubemaps/thick_clouds.ktx"));
    std::unique_ptr<Cubemap> cubemap2 = std::unique_ptr<Cubemap>(new Cubemap("/textures/cubemaps/sunset.ktx"));
    Skybox skybox(cubemap0.get());

    Sun sun(Vec3f(0,-0.5,1), Vec3f(100,100,100), Color::WHITE, 1.0f, new ShadowInfo(11, 1.0f, VK_FILTER_LINEAR, 150.0f));
    sun.toggleLightShafts();
    //sun.getDirLight().toggleActive();
    //sun.getDirLight().addComponent(new CRotate(0.4f));
    //sun.getDirLight().toggleType();

    //DirectionalLight directionalLight2(Color::RED, 1.0f, Vec3f(0, -1, 1), new ShadowInfo(10, 1.0f, VK_FILTER_LINEAR, 150.0f));
    //DirectionalLight directionalLight3(Color::WHITE, 0.3f, Vec3f(1, -1, 0), new ShadowInfo(10, 1.0f, VK_FILTER_LINEAR, 150.0f));
    //DirectionalLight directionalLight4(Color::WHITE, 0.3f, Vec3f(-1, -1, 0), new ShadowInfo(10, 1.0f, VK_FILTER_LINEAR, 150.0f));

    PointLight pointLight(Color::WHITE, 200, Vec3f(0, 0, 0.5f), Point3f(0, -4, -5), new ShadowInfo(9));
    //pointLight.addComponent(new CMove(Vec3f(20,0,0), 0.5f));
    pointLight.toggleActive();
    //pointLight.toggleType();
    //PointLight pointLight2(Color::BLUE, 20, Vec3f(0, 0, 0.8f), Point3f(0, 4, -10));
    //PointLight pointLight3(Color::GREEN, 20, Vec3f(0, 0, 0.8f), Point3f(10, 4, -20));
    //PointLight pointLight4(Color(1,1,0), 20, Vec3f(0, 0, 0.8f), Point3f(0, 4, -20));

    SpotLight spotLight(Color::WHITE, 100, 90.0f, Vec3f(0.0f, 0.0f, 0.3f), Point3f(-8, -5, 0), Vec3f(1, 0, 0), new ShadowInfo(8));
    spotLight.toggleActive();
    //spotLight.toggleType();
    //spotLight.addComponent(new CRotate(0.3f));
    //spotLight.addComponent(new CMove(Vec3f(0,0,20), 1.0f));

    //skybox.setCubemap(pointLight.getShadowMapTex());

// 1000 Lights Test
#if 0
    const int numLights = 1000;
    std::unique_ptr<PointLight> pointLights2[numLights];
    int startX = -100, startZ = 40;
    for (int i = 0; i < numLights; i++)
    {
        pointLights2[i] = std::unique_ptr<PointLight>(new PointLight(Color(Random::randomFloat(),
                                                        Random::randomFloat(), 
                                                        Random::randomFloat()), 
                                                        50.0f, Vec3f(.0f, .0f, 0.8f), 
                                                        Point3f(startX, 0, startZ), nullptr));
        pointLights2[i]->toggleType();
        startX += 15;
        if (i % 20 == 0)
        {
            startZ -= 20;
            startX = -100;
        }
    }
#endif

    Texture grass("/textures/grass.dds");
    BasicMaterial planeMat(&grass);
    planeMat.setFloat("uvScale", 15.0f);
    planeMat.setFloat("dispScale", 0.04f);
    planeMat.setFloat("dispBias", -0.03f);
    planeMat.setFloat("specularIntensity", 0.5f);
    
    Texture bricks("/textures/bricks2.dds");
    Texture brickNormal("/textures/bricks2Normal.dds");
    Texture brickDisp("/textures/bricks2Disp.dds");
    BasicMaterial bricksMat(&bricks);
    bricksMat.setFloat("uvScale", 5.0f);
    bricksMat.setTexture("normalMap", &brickNormal);
    bricksMat.setTexture("dispMap", &brickDisp);
    bricksMat.setFloat("dispScale", 0.04f);
    bricksMat.setFloat("dispBias", -0.03f);
    bricksMat.setFloat("specularIntensity", 0.5f);
    
    Texture rockTexture("/textures/white.dds");
    BasicMaterial rockMaterial(&rockTexture);

    //Texture ktxTexture("pattern.dds");
    //Material ktxMaterial(&ktxTexture);

    ForwardShader reflect("Reflect", "/shaders/reflect", PipelineType::Basic);
    Material ktxMaterial(&reflect);
    ktxMaterial.setTexture("SamplerCubeMap", cubemap0.get());

    Quad plane(Transform(Point3f(0,-7,0), Vec3f(1000,1000,1), Quatf::rotationX(Mathf::deg2Rad(90))), &planeMat);
    Quad plane2(Transform(Point3f(-30, -3, 30), Vec3f(10, 10, 1), Quatf::rotationX(Mathf::deg2Rad(90))), &bricksMat);

// Huge Amount of Objects Test
#if 0
    const int numCubes = 10000;
    std::unique_ptr<Cube> cubes[numCubes];
    float x = 0, z = 0;
    for (int i = 0; i < numCubes; i++)
    {
        cubes[i] = std::unique_ptr<Cube>(new Cube(Transform(Point3f(x * 3, 0.0f, -z)), &rockMaterial));
        x++;
        if (i % 50 == 0){
            x = 0;
            z = z + 3;
        }
        cubes[i]->addComponent(new CRotate(1));
    }
#endif

    Mesh mesh("/models/monkey_no_uv.obj");
    //Mesh mesh("res/models/monkey_hi_res.obj");
    
    Renderable node3("Monkey", &mesh, &ktxMaterial, Transform(Point3f(-20,0,0), Vec3f(2,2,2), Quatf::rotationY(Mathf::deg2Rad(180))));
    node3.addComponent(new CRotate(1));
    //node3.addComponent(new CMove(Vec3f(15,0,0), 30.0f));

    float scale = 5.0f;
    Renderable node(&mesh, &ktxMaterial, Transform(Point3f(0,0,0), Vec3f(scale, scale, scale)));

    node.addComponent(new LookAt(&cam));
    //node.setParent(&node3);

    //Texture sponge("sponge.dds");
    //Material spongeMat(&sponge);
    //Quad quad(Transform(Point3f(0, 15, -50), Vec3f(10, 10, 10)), &spongeMat);
    //quad.addComponent(new LookAt(&cam));

    //Texture obj1Tex("Grass_01.dds");
    //Texture obj1Normal("Grass_01.dds");
    //Mesh obj1Mesh("res/models/Grass_02.obj");
    //Mesh obj2Mesh("res/models/Grass_03.obj");
    //
    //Material obj1Mat(&obj1Tex);
    //obj1Mat.setTexture("normalMap", &obj1Normal);
    //
    //Renderable grassBack("grass", &obj2Mesh, &obj1Mat, Transform(Point3f(15, -7, -45), Vec3f(5, 5, 5)));
    //Renderable grassTile("grass2", &obj1Mesh, &obj1Mat, Transform(Point3f(-15, -7.5f, -15), Vec3f(3, 3, 3)));

    Mesh crateMesh("/models/crate.obj");
    Texture crateTex("/textures/crate.dds");
    Texture crateTexNormal("/textures/crateNormal.dds");
    BasicMaterial crateMat(&crateTex);
    crateMat.setTexture("normalMap", &crateTexNormal);
    Renderable crate("TEST", &crateMesh, &crateMat, Transform(Point3f(3,0,0), Vec3f(0.01f,0.01f,0.01f)));
    
    crate.setParent(&node3);

    // Spawn Boxes
    std::vector<std::unique_ptr<Renderable>> boxes;
    {
        Input::attachFunc(KeyCodes::B, [&] {
            Point3f spawnPosition = cam.getWorldPosition();
            std::unique_ptr<Renderable> box(new Renderable("TEST", &crateMesh, &crateMat, Transform(spawnPosition, Vec3f(0.01f, 0.01f, 0.01f))));
            boxes.push_back(std::move(box));
        }, Input::KEY_PRESSED);
    }

    Renderable crate2(&crateMesh, &crateMat, Transform(Point3f(6.0f, -6.0f, -3.0f), Vec3f(0.01f, 0.01f, 0.01f)));
    //Renderable crate3(&crateMesh, &crateMat, Transform(Point3f(-6.0f, -6.0f, -3.0f), Vec3f(0.01f, 0.01f, 0.01f)));
    //Renderable crate4(&crateMesh, &crateMat, Transform(Point3f(0.0f, -6.0f, -12.0f), Vec3f(0.01f, 0.01f, 0.01f)));

    //Camera& shadowCam = *sun.getDirLight().getShadowCamera();
    //Frustum& frustum = shadowCam.getFrustum();

#if 1
    ForwardShader wireframe("WireframeLocal", "/shaders/solid", PipelineType::Wireframe);
    Mesh sphereMesh("/models/sphere.obj");
    Material boundingBoxMaterial(&wireframe);
    std::vector<std::unique_ptr<Renderable>> sphereColliders;
    Input::attachFunc(KeyCodes::F, [&] {
        static bool showColliders = true;
        sphereColliders.clear();
        if (showColliders)
        {
            std::vector<Renderable*> renderables;
            for (auto& renderable : SceneGraph::getRenderables())
                renderables.push_back(renderable);

            for (auto& renderable : renderables)
            {
                SphereCollider* collider = renderable->getComponent<SphereCollider>();
                float meshRadius = collider->getRadius();
                float meshRadius2 = sphereMesh.getDimension().max.maxValue();
                float scale = meshRadius / meshRadius2;
                Point3f colliderPos = renderable->getWorldTransform().position;
                std::unique_ptr<Renderable> sphere = std::unique_ptr<Renderable>(new Renderable(&sphereMesh, &boundingBoxMaterial,
                    Transform(colliderPos, Vec3f(scale, scale, scale))));
                sphereColliders.push_back(std::move(sphere));
            }
        }
        showColliders = !showColliders;
    }, Input::KEY_PRESSED);
#endif


#ifdef FREETYPE_LIB
    // GUI Stuff

    // Depth-Texture shown in the HUD (TOGGLE with "M")
    Texture* depthTex = sun.getDirLight().getShadowMapTex();
    GUIImage shadowMap(depthTex, Vec2f(), 400, Color::WHITE, Anchor::BottomRight);
    shadowMap.setActive(false);
    shadowMap.setLocalPos(Vec2f(-shadowMap.getWidth(), -shadowMap.getHeight()));
    Input::attachFunc(KeyCodes::M, [&] { shadowMap.toggleActive(); }, Input::KEY_PRESSED);

    Node fpsGUI("fpsGUI");

    GUIText fps("FPS", Vec2f(5, 30), Color(1,0,1,1));
    GUI* gui = new GUI({ &fps, &shadowMap });

    fpsGUI.addComponent(gui);

    const Font* font = FontManager::addFont("DebugFont", "/fonts/arial/arial.ttf", 24);

    // DEBUG MENU
    DebugMenu debugMenu(font);
    {
        debugMenu.addButton("Settings", nullptr);
        debugMenu.addButton("Rendering-Mode", nullptr, "Settings");
        debugMenu.addButton("Wireframe", [&] { renderer.setRenderingMode(ERenderingMode::WIREFRAME); }, "Settings-Rendering-Mode");
        debugMenu.addButton("Solid",     [&] { renderer.setRenderingMode(ERenderingMode::SOLID); }, "Settings-Rendering-Mode");
        debugMenu.addButton("Unlit",     [&] { renderer.setRenderingMode(ERenderingMode::UNLIT); }, "Settings-Rendering-Mode");
        debugMenu.addButton("Lit",       [&] { renderer.setRenderingMode(ERenderingMode::LIT); }, "Settings-Rendering-Mode");
        debugMenu.addButton("ToggleNormalMaps", [&] { renderer.toggleNormalMapRendering(); }, "Settings-Rendering-Mode-Lit");
        debugMenu.addButton("ToggleDispMaps", [&] { renderer.toggleDispMapRendering(); }, "Settings-Rendering-Mode-Lit");

        debugMenu.addButton("Albedo", [&] { renderer.toggleRenderAlbedo(); }, "Settings-Rendering-Mode-Lit");
        debugMenu.addButton("Normal", [&] { renderer.toggleRenderNormals(); }, "Settings-Rendering-Mode-Lit");

        debugMenu.addSliderButton("Ambient", [&](float value) { renderer.setAmbientIntensity(value); }, "Settings");
        debugMenu.addSliderButton("Time-Scale", [&](float value) { renderer.setTimeScale(value * 8 - 4); }, "Settings");
        debugMenu.addButton("Toggle Type", [&] {
            for (auto obj : SceneGraph::getRenderables())
                obj->toggleType();
        }, "Settings");
        debugMenu.addButton("PostProcessing", nullptr, "Settings");
        debugMenu.addButton("Toggle Post-Process", [&] { renderer.togglePostProcessing(); }, "Settings-PostProcessing");
        debugMenu.addSliderButton("Resolution Mod", [&](float value) { renderer.setResolutionMod(value); }, "Settings-PostProcessing");
        if (Shader::exists("FXAA"))
        {
            debugMenu.addButton("FXAA", nullptr, "Settings-PostProcessing");
            debugMenu.addButton("Toggle", [&] { Shader::get("FXAA")->toggleActive(); }, "Settings-PostProcessing-FXAA");
            debugMenu.addSliderButton("SpanMax", [&](float value) { Shader::get("FXAA")->setFloat("spanMax", value * 10); }, "Settings-PostProcessing-FXAA");
            debugMenu.addSliderButton("ReduceMin", [&](float value) { Shader::get("FXAA")->setFloat("reduceMin", value / 10); }, "Settings-PostProcessing-FXAA");
            debugMenu.addSliderButton("ReduceMax", [&](float value) { Shader::get("FXAA")->setFloat("reduceMul", value); }, "Settings-PostProcessing-FXAA");
        }
        if (Shader::exists("Tonemap"))
        {
            debugMenu.addButton("Tonemap", nullptr, "Settings-PostProcessing");
            debugMenu.addButton("Toggle", [&] { Shader::get("Tonemap")->toggleActive(); }, "Settings-PostProcessing-Tonemap");
            debugMenu.addSliderButton("Exposure", [&](float value) { Shader::get("Tonemap")->setFloat("exposure", 5 * value); }, "Settings-PostProcessing-Tonemap");
            debugMenu.addSliderButton("Gamma", [&](float value) { Shader::get("Tonemap")->setFloat("gamma", 3 * value); }, "Settings-PostProcessing-Tonemap");
        }
        if (Shader::exists("LightShafts"))
        {
            debugMenu.addButton("LightShafts", nullptr, "Settings-PostProcessing");
            debugMenu.addButton("Toggle", [&] { sun.toggleLightShafts(); }, "Settings-PostProcessing-LightShafts");
            debugMenu.addSliderButton("Scale", [&](float value) { Shader::get("LightShafts#RadialBlur")->setFloat("radialBlurScale", value); }, "Settings-PostProcessing-LightShafts");
            debugMenu.addSliderButton("Strength", [&](float value) { Shader::get("LightShafts#RadialBlur")->setFloat("radialBlurStrength", value * 2); }, "Settings-PostProcessing-LightShafts");
            debugMenu.addButton("Color", nullptr, "Settings-PostProcessing-LightShafts");
            debugMenu.addSliderButton("Red", [&](float value) { sun.setRedChannel(value); }, "Settings-PostProcessing-LightShafts-Color");
            debugMenu.addSliderButton("Green", [&](float value) { sun.setGreenChannel(value); }, "Settings-PostProcessing-LightShafts-Color");
            debugMenu.addSliderButton("Blue", [&](float value) { sun.setBlueChannel(value); }, "Settings-PostProcessing-LightShafts-Color");
        }
        if (Shader::exists("Contrast"))
        {
            debugMenu.addButton("Contrast", nullptr, "Settings-PostProcessing");
            debugMenu.addButton("Toggle", [&] { Shader::get("Contrast")->toggleActive(); }, "Settings-PostProcessing-Contrast");
            debugMenu.addSliderButton("Strength", [&](float value) { Shader::get("Contrast")->setFloat("contrast", value); }, "Settings-PostProcessing-Contrast");
        }
        if (Shader::exists("Bloom"))
        {
            debugMenu.addButton("Bloom", nullptr, "Settings-PostProcessing");
            debugMenu.addButton("Toggle", [&] { Shader::get("Bloom")->toggleActive(); }, "Settings-PostProcessing-Bloom");
            debugMenu.addSliderButton("Combine Strength", [&](float value) { Shader::get("Bloom")->setFloat("combineStrength", value * 2); }, "Settings-PostProcessing-Bloom");
            debugMenu.addSliderButton("Blur Strength", [&](float value) {
                Shader::get("Bloom#HBlur1")->setFloat("blurStrength", value * 10);
                Shader::get("Bloom#VBlur1")->setFloat("blurStrength", value * 10);
            }, "Settings-PostProcessing-Bloom");
        }
        if (Shader::exists("DepthOfField"))
        {
            debugMenu.addButton("DOA", nullptr, "Settings-PostProcessing");
            debugMenu.addButton("Toggle", [&] { Shader::get("DepthOfField")->toggleActive(); }, "Settings-PostProcessing-DOA");
            debugMenu.addSliderButton("Focus-Treshold", [&](float value) { Shader::get("DepthOfField")->setFloat("focusThreshold", value * 500.0f); }, "Settings-PostProcessing-DOA");
            debugMenu.addSliderButton("Transition-Distance", [&](float value) { Shader::get("DepthOfField")->setFloat("transitionDistance", value * 100.0f); }, "Settings-PostProcessing-DOA");
        }

        //---------------------------------------------------------------------------
        //  Lights
        //---------------------------------------------------------------------------
        debugMenu.addButton("Lights", nullptr);
        debugMenu.addButton("Dir-Light", nullptr, "Lights");
        debugMenu.addButton("Shadows", nullptr, "Lights-Dir-Light");
        debugMenu.addButton("Toggle", [&] { sun.getDirLight().toggleShadows(); }, "Lights-Dir-Light-Shadows");
        debugMenu.addSliderButton("Shadow-Softness", [&](float value) { sun.getDirLight().setShadowBlurScale(value * 2); }, "Lights-Dir-Light-Shadows");
        debugMenu.addSliderButton("MinVariance", [&](float value) { sun.getDirLight().setMinVariance(value * 0.00001f); }, "Lights-Dir-Light-Shadows");
        debugMenu.addSliderButton("LinStep", [&](float value) { sun.getDirLight().setLinStep(value); }, "Lights-Dir-Light-Shadows");
        debugMenu.addButton("Toggle Type", [&] { sun.getDirLight().toggleType(); }, "Lights-Dir-Light");
        debugMenu.addSliderButton("Intensity", [&](float value) { sun.getDirLight().setIntensity(value); }, "Lights-Dir-Light");
        debugMenu.addButton("Rotation", nullptr, "Lights-Dir-Light");
        debugMenu.addSliderButton("Yaw", [&](float value) { sun.getDirLight().setRotation(Vec3f(sun.getDirLight().getRotation().toEulerAngles().x(), value * 360, 0).toQuaternion()); }, "Lights-Dir-Light-Rotation");
        debugMenu.addSliderButton("Pitch", [&](float value) { sun.getDirLight().setRotation(Vec3f(value * 360, sun.getDirLight().getRotation().toEulerAngles().y(), 0).toQuaternion()); }, "Lights-Dir-Light-Rotation");
        debugMenu.addButton("Color", nullptr, "Lights-Dir-Light");
        debugMenu.addSliderButton("Red", [&] (float value) { sun.getDirLight().getColor().r() = value; }, "Lights-Dir-Light-Color");
        debugMenu.addSliderButton("Green", [&](float value) { sun.getDirLight().getColor().g() = value; }, "Lights-Dir-Light-Color");
        debugMenu.addSliderButton("Blue", [&](float value) { sun.getDirLight().getColor().b() = value; }, "Lights-Dir-Light-Color");
        debugMenu.addSliderButton("ShadowDistance", [&](float value) { sun.getDirLight().setShadowDistance(value * 1000); }, "Lights-Dir-Light");
        debugMenu.addButton("Point-Light", nullptr, "Lights");

        debugMenu.addButton("Toggle Active", [&] { pointLight.toggleActive(); }, "Lights-Point-Light");
        debugMenu.addButton("Toggle Static/Dynamic", [&] { pointLight.toggleType(); }, "Lights-Point-Light");
        debugMenu.addSliderButton("Intensity", [&](float value) { pointLight.setIntensity(value * 500); }, "Lights-Point-Light");
        debugMenu.addButton("Attenuation", nullptr, "Lights-Point-Light");
        debugMenu.addSliderButton("Constant", [&](float value) {pointLight.setConstant(value * 5); }, "Lights-Point-Light-Attenuation");
        debugMenu.addSliderButton("Linear", [&](float value) {pointLight.setLinear(value * 5); }, "Lights-Point-Light-Attenuation");
        debugMenu.addSliderButton("Exponent", [&](float value) {pointLight.setExponent(value * 5); }, "Lights-Point-Light-Attenuation");
        debugMenu.addButton("Color", nullptr, "Lights-Point-Light");
        debugMenu.addSliderButton("Red", [&](float value) {pointLight.getColor().r() = value; }, "Lights-Point-Light-Color");
        debugMenu.addSliderButton("Green", [&](float value) {pointLight.getColor().g() = value; }, "Lights-Point-Light-Color");
        debugMenu.addSliderButton("Blue", [&](float value) {pointLight.getColor().b() = value; }, "Lights-Point-Light-Color");
        debugMenu.addButton("Position", nullptr, "Lights-Point-Light");
        debugMenu.addSliderButton("X", [&](float value) {pointLight.getLocalPosition().x() = value * 100 - 50; }, "Lights-Point-Light-Position");
        debugMenu.addSliderButton("Y", [&](float value) {pointLight.getLocalPosition().y() = value * 100 - 50; }, "Lights-Point-Light-Position");
        debugMenu.addSliderButton("Z", [&](float value) {pointLight.getLocalPosition().z() = value * 100 - 50; }, "Lights-Point-Light-Position");

        debugMenu.addButton("SpotLight", nullptr, "Lights");
        debugMenu.addButton("Toggle Active", [&] { spotLight.toggleActive(); }, "Lights-SpotLight");
        debugMenu.addButton("Toggle Dynamic/Static", [&]() { spotLight.toggleType(); }, "Lights-SpotLight");
        debugMenu.addSliderButton("Intensity", [&](float value) { spotLight.setIntensity(value * 500); }, "Lights-SpotLight");
        debugMenu.addButton("Color", nullptr, "Lights-SpotLight");
        debugMenu.addSliderButton("Red", [&](float value) {spotLight.getColor().r() = value; }, "Lights-SpotLight-Color");
        debugMenu.addSliderButton("Green", [&](float value) {spotLight.getColor().g() = value; }, "Lights-SpotLight-Color");
        debugMenu.addSliderButton("Blue", [&](float value) {spotLight.getColor().b() = value; }, "Lights-SpotLight-Color");
        debugMenu.addSliderButton("FOV", [&](float value) { spotLight.setFOV(value * 180.0f); }, "Lights-SpotLight");
        debugMenu.addButton("Position", nullptr, "Lights-SpotLight");
        debugMenu.addSliderButton("X", [&](float value) {spotLight.getTransform().position.x() = value * 100 - 50; }, "Lights-SpotLight-Position");
        debugMenu.addSliderButton("Y", [&](float value) {spotLight.getTransform().position.y() = value * 100 - 50; }, "Lights-SpotLight-Position");
        debugMenu.addSliderButton("Z", [&](float value) {spotLight.getTransform().position.z() = value * 100 - 50; }, "Lights-SpotLight-Position");
        debugMenu.addButton("Rotation", nullptr, "Lights-SpotLight");
        debugMenu.addSliderButton("X", [&](float value) {spotLight.getTransform().rotation = Quatf::euler(value * 360, 0, 0); }, "Lights-SpotLight-Rotation");
        debugMenu.addSliderButton("Y", [&](float value) {spotLight.getTransform().rotation = Quatf::euler(0, value * 360, 0); }, "Lights-SpotLight-Rotation");

        debugMenu.addButton("Shadows", nullptr);
        debugMenu.addButton("Toggle Shadows", [&]  {renderer.toggleShadows(); }, "Shadows");
        //debugMenu.addSliderButton("Depth-Bias-Constant", [&](float value) { renderer.getShadowRenderer()->setDepthBiasConstant(value * 100); }, "Shadows");
        //debugMenu.addSliderButton("Depth-Bias-Slope", [&](float value) { renderer.getShadowRenderer()->setDepthBiasSlope(value * 100); }, "Shadows");


        debugMenu.addButton("Camera", nullptr);
        debugMenu.addButton("Ortho", [&] { cam.setRenderingMode(Camera::ORTHOGRAPHIC); }, "Camera");
        debugMenu.addButton("Perspective", [&] { cam.setRenderingMode(Camera::PERSPECTIVE); }, "Camera");
        debugMenu.addSliderButton("FOV", [&](float value) { cam.setFOV(value * 180); }, "Camera");
        debugMenu.addSliderButton("zFar", [&](float value) { cam.setZFar(value * 1000.0f); }, "Camera");
        debugMenu.addSliderButton("zNear", [&](float value) { cam.setZNear(value * 100.0f); }, "Camera");
        debugMenu.addSliderButton("Left", [&](float value) { cam.setLeft(-value * 100); }, "Camera");
        debugMenu.addSliderButton("Right", [&](float value) { cam.setRight(value * 100); }, "Camera");
        debugMenu.addSliderButton("Top", [&](float value) { cam.setTop(value * 100); }, "Camera");
        debugMenu.addSliderButton("Bottom", [&](float value) { cam.setBottom(-value * 100); }, "Camera");

        //debugMenu.addButton("ShadowCamera", nullptr);
        //debugMenu.addButton("Ortho", [&] { shadowCam.setRenderingMode(Camera::ORTHOGRAPHIC); }, "ShadowCamera");
        //debugMenu.addButton("Perspective", [&] { shadowCam.setRenderingMode(Camera::PERSPECTIVE); }, "ShadowCamera");
        //debugMenu.addSliderButton("FOV", [&](float value) { shadowCam.setFOV(value * 180); }, "ShadowCamera");
        //debugMenu.addSliderButton("zFar", [&](float value) { shadowCam.setZFar(value * 1000.0f); }, "ShadowCamera");
        //debugMenu.addSliderButton("zNear", [&](float value) { shadowCam.setZNear(value * 100.0f); }, "ShadowCamera");
        //debugMenu.addSliderButton("Left", [&](float value) { shadowCam.setLeft(-value * 100); }, "ShadowCamera");
        //debugMenu.addSliderButton("Right", [&](float value) { shadowCam.setRight(value * 100); }, "ShadowCamera");
        //debugMenu.addSliderButton("Top", [&](float value) { shadowCam.setTop(value * 100); }, "ShadowCamera");
        //debugMenu.addSliderButton("Bottom", [&](float value) { shadowCam.setBottom(-value * 100); }, "ShadowCamera");

        //---------------------------------------------------------------------------
        //  Objects
        //---------------------------------------------------------------------------
        debugMenu.addButton("Objects", nullptr);

        debugMenu.addButton("Monkey", nullptr, "Objects");
        debugMenu.addButton("Toggle Active", [&] { node.toggleActive(); }, "Objects-Monkey");
        debugMenu.addButton("Transform", nullptr, "Objects-Monkey");
        debugMenu.addSliderButton("Scale", [&](float value) { node.getTransform().scale = Vec3f(value * 20, value * 20, value * 20); }, "Objects-Monkey-Transform");
        debugMenu.addButton("Material", nullptr, "Objects-Monkey");
        debugMenu.addSliderButton("Specular-Intensity", [&](float value) { node.getMaterial()->setFloat("specularIntensity", value * 5); }, "Objects-Monkey-Material");
        debugMenu.addSliderButton("Specular-Power", [&](float value) { node.getMaterial()->setFloat("specularPower", value * 100); }, "Objects-Monkey-Material");
       
        debugMenu.addButton("Monkey2", nullptr, "Objects");
        debugMenu.addButton("Toggle Dynamic/Static", [&]() { node3.toggleType(); }, "Objects-Monkey2");
        debugMenu.addButton("Transform", nullptr, "Objects-Monkey2");
        debugMenu.addSliderButton("Scale", [&](float value) { node3.getTransform().scale = Vec3f(value * 20, value * 20, value * 20); }, "Objects-Monkey2-Transform");

        //---------------------------------------------------------------------------
        //  Materials
        //---------------------------------------------------------------------------
        debugMenu.addButton("Materials", nullptr);
        debugMenu.addButton("KTX", nullptr, "Materials");
        debugMenu.addSliderButton("Alpha", [&](float value) { ktxMaterial.setColor("color", Color(1, 1, 1, value)); }, "Materials-KTX");
        debugMenu.addSliderButton("Texture-Scale", [&](float value) { ktxMaterial.setFloat("uvScale", value * 10); }, "Materials-KTX");
        debugMenu.addButton("Plane", nullptr, "Materials");
        debugMenu.addSliderButton("Texture-Scale", [&](float value) { planeMat.setFloat("uvScale", value * 20); }, "Materials-Plane");
        debugMenu.addSliderButton("Specular-Intensity", [&](float value) { planeMat.setFloat("specularIntensity", value * 5); }, "Materials-Plane");
        debugMenu.addSliderButton("Specular-Power", [&](float value) { planeMat.setFloat("specularPower", value * 100); }, "Materials-Plane");
        //debugMenu.addButton("Bricks", nullptr, "Materials");
        //debugMenu.addSliderButton("Texture-Scale", [&](float value) { bricksMat.setFloat("uvScale", value * 20); }, "Materials-Bricks");
        //debugMenu.addSliderButton("Displacement-Scale", [&](float value) { bricksMat.setFloat("dispScale", value * 0.1f); }, "Materials-Bricks");
        //debugMenu.addSliderButton("Displacement-Bias", [&](float value) { bricksMat.setFloat("dispBias", -value  * 0.1f); }, "Materials-Bricks");
        //debugMenu.addSliderButton("Specular-Intensity", [&](float value) { bricksMat.setFloat("specularIntensity", value * 5); }, "Materials-Bricks");
        //debugMenu.addSliderButton("Specular-Power", [&](float value) { bricksMat.setFloat("specularPower", value * 100); }, "Materials-Bricks");

        if (Shader::exists("FOG"))
        {
            debugMenu.addButton("FOG", nullptr);
            debugMenu.addButton("Toggle", [&] { renderer.toggleFOG(); }, "FOG");
            debugMenu.addSliderButton("Density", [&](float value) { renderer.setFOGDensity(value * 0.01f); }, "FOG");
            debugMenu.addSliderButton("Gradient", [&](float value) { renderer.setFOGGradient(value * 10.0f + 1); }, "FOG");
            debugMenu.addButton("Color", nullptr, "FOG");
            debugMenu.addSliderButton("Red", [&](float value) { const Color& fogColor = renderer.getFogColor();
            renderer.setFogColor(Color(value, fogColor.g(), fogColor.b())); }, "FOG-Color");
            debugMenu.addSliderButton("Green", [&](float value) {const Color& fogColor = renderer.getFogColor();
            renderer.setFogColor(Color(fogColor.r(), value, fogColor.b()));  }, "FOG-Color");
            debugMenu.addSliderButton("Blue", [&](float value) {const Color& fogColor = renderer.getFogColor();
            renderer.setFogColor(Color(fogColor.r(), fogColor.g(), value)); }, "FOG-Color");

            debugMenu.addSliderButton("Skybox-Upper-Limit", [&](float value) { skybox.setFogUpperLimit(value * 10); }, "FOG");
        }

        // Skybox loading
        {
            debugMenu.addButton("Skybox", nullptr);
            debugMenu.addButton("SecondCubemap", nullptr, "Skybox");
            debugMenu.addSliderButton("Blend-Factor", [&](float value) { skybox.setBlendFactor(value); }, "Skybox-SecondCubemap");

            debugMenu.addButton("Cubemap 0", [&] {
                skybox.setSecondCubemap(cubemap0.get());
            }, "Skybox-SecondCubemap");

            debugMenu.addButton("Cubemap 1", [&] {
                skybox.setSecondCubemap(cubemap1.get());
            }, "Skybox-SecondCubemap");
            debugMenu.addButton("Cubemap 2", [&] {
                skybox.setSecondCubemap(cubemap2.get());
            }, "Skybox-SecondCubemap");

            debugMenu.addButton("Cubemap 0", [&] {
                skybox.setCubemap(cubemap0.get());
            }, "Skybox");

            debugMenu.addButton("Cubemap 1", [&] {
                skybox.setCubemap(cubemap1.get());
            }, "Skybox");
            debugMenu.addButton("Cubemap 2", [&] {
                skybox.setCubemap(cubemap2.get());
            }, "Skybox");
        }
    }
#endif

    uint64_t fpsTimer = 0; // fps timer (one second interval)
    uint64_t lastFrameTime = 0;
    
    while (isRunning)
    {
        TimeManager::update();

        fpsTimer += Time::getDelta();
        if (fpsTimer > Time::SECOND)
        {
            //std::cout << "DELTA: " << Time::getDelta() / (float)Time::MILLISECOND << " ms" << std::endl;
            std::string windowTitle = "FPS: " + std::to_string(Time::getFPS()) + " (" + std::to_string(1000.0f / Time::getFPS()) + " ms)";
            //window.setWindowText(windowTitle.c_str());
            #ifdef FREETYPE_LIB
                fps.setText(windowTitle);
            #endif
            fpsTimer -= Time::SECOND;
        }

        update(static_cast<float>(Time::getDeltaSeconds()));
        renderer.draw();

        if (!window.update())
            isRunning = false;
    }

    renderer.shutdown();
}

void Application::update(float delta)
{
    if (Input::getKeyDown(KeyCodes::ESCAPE))
       isRunning = false;

    InputManager::update(delta);
    renderer.update(delta);
}
