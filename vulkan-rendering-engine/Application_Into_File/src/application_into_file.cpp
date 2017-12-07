#include "application_into_file.h"

#include "time/time_manager.h"
#include "Input/input_manager.h"

using namespace Pyro;


//---------------------------------------------------------------------------
//  Application class
//---------------------------------------------------------------------------

#if USE_WINDOW
Application::Application(int width, int height)
    : window(width, height), renderer(&window)
{
    run();
}
#else
Application::Application(int width, int height)
    : renderer(Vec2ui(width, height))
{
    run();
}
#endif

// Will be set to true if rendering to files has been finished
static bool renderingFinished = false;

// The renderer renders every call to draw() one frame of the current scene
// init() is called once the scene gets initialized
// update() is called every frame
class MyScene : public Scene
{
    // Need those two references for use the update() method
    RenderingEngine* renderer;
    Camera* cam;

    Point3f pointOfInterest = Point3f(0, 5, 0); // Where the camera should look at
    int radius = 15;                            // Distance from the camera to the Point of Interest
    int numCycles = 5;                          // Number of images to be rendered
    int degree = 0;                             // Angle of the camera around the POI

public:
    MyScene() : Scene("MyScene") {}
    ~MyScene() {}

    // Build up / initialize the scene
    void init(RenderingEngine* r) override
    {
        // NOTICE: All ENGINE-objects created with "new" are managed by the engine and will be deleted automatically

        // Save a reference because we need the renderer in update()
        renderer = r;

        // Default-Values for zNear: 0.1f , zFar: 1000.0f
        cam = new Camera(Transform(Point3f(0, 3, 10)));

        // The renderer needs always a camera
        renderer->setCamera(cam);

        // Set some render-settings. Image-Based-Lighting (IBL) is only of interest for physically based rendering
        renderer->setClearColor(Color::BLACK);
        renderer->setAmbientIntensity(0.2f);
        renderer->setUseIBL(false);

#if !USE_WINDOW
        // Output in HD
        renderer->setFinalResolution(Vec2ui(1280, 720));
        // With this function you can render in higher / lower resolutions than the output -> Supersampling
        //renderer.setResolutionMod(2.0f);
#else
        // Switch between "Maya" and "FPS" mode with button "1" or "2"
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));
#endif

        // Virtual Paths begins with '/' e.g.  "/models/..."
        //VFS::mount("models", "res/models"); 

        //---------------------------------------------------------------------------
        //  Lights
        //---------------------------------------------------------------------------

        // First parameter: Shadowmap - resolution => In this case 2^11 = 2048x2048
        ShadowInfo* shadowInfo = new ShadowInfo(11, 0.5f, 100.0f);
        DirectionalLight* dirLight = new DirectionalLight(Color::WHITE, 3.0f, Vec3f(-1, -1, 0), shadowInfo);

        //---------------------------------------------------------------------------
        //  Meshes
        //---------------------------------------------------------------------------

        // Load a mesh from a file (the mesh itself can have materials and textures which will be loaded aswell)
        //auto mesh = MESH("/models/sponza/sponza.obj");
        auto mesh = MESH("/models/cat/cat.obj");
        auto cube = MESH("/models/crate.obj");

        //---------------------------------------------------------------------------
        //  Textures
        //---------------------------------------------------------------------------

        // Custom sampler, but not necessary
        std::shared_ptr<Sampler> sampler(new Sampler(1.0f, FILTER_LINEAR, FILTER_LINEAR, MIPMAP_MODE_LINEAR));

        auto tex_grass = TEXTURE({ "/textures/grass.dds", sampler });
        auto tex_rock = TEXTURE({ "/textures/rock.jpg", sampler });

        //---------------------------------------------------------------------------
        //  Materials
        //---------------------------------------------------------------------------

        // Every Material is associated with a shader. Basic materials just take one texture (diffuse one).
        // But they have normal-maps / displacement-maps associated with it aswell.
        auto grassMat = PBRMATERIAL(tex_grass);
        // Set some shader-parameters. Of course they have default-values.
        grassMat->setMatMetallic(0.0f);
        grassMat->setMatRoughness(0.0f);
        grassMat->setMatUVScale(15.0f);
        // If i want to change the diffuse/normalmap i do this:
        //grassMat->setMatDiffuseTexture(diffuseTex);
        //grassMat->setMatNormalMap(normalMap);

        auto rockMat = PBRMATERIAL(tex_rock);

        auto cubemap0 = CUBEMAP("/textures/cubemaps/tropical_sunny_day.dds");

        //---------------------------------------------------------------------------
        //  Objects
        //---------------------------------------------------------------------------
        Skybox* skybox = new Skybox(cubemap0);

        // Predefines meshes (simple quads)
        Quad* plane = new Quad(grassMat, Transform(Point3f(0, -7, 0), Vec3f(100, 100, 1), Quatf::rotationX(Mathf::deg2Rad(90))));
        Quad* plane2 = new Quad(rockMat, Transform(Point3f(0, 0, -10), Vec3f(10, 10, 10), Quatf::rotationY(Mathf::deg2Rad(180))));

        // Renderable with a mesh WITH materials
        float scale = 3.0f;
        Renderable* node = new Renderable(mesh, Transform(Point3f(0, 0, 0), Vec3f(scale, scale, scale), Quatf::rotationY(Mathf::deg2Rad(0))));

        // Renderable with a mesh + separate material
        Renderable* box = new Renderable(cube, rockMat, Transform(Point3f(0, 10, 0), Vec3f(0.01f, 0.01f, 0.01f)));
    }

    // Update the scene
    void update(float delta) override
    {
#if !USE_WINDOW
        static int i = 0;
        Vec2ui resolution = renderer->get3DRenderResolution();
        Logger::Log("Render-Resolution: [" + std::to_string(resolution.x()) + "," + std::to_string(resolution.y()) + "]");

        // path starts at the visual studio's project path
        std::string filePath = "test_#" + std::to_string(i) + ".png";

        // This callback will be called once the renderer has finished rendering (in the draw() method).
        // It must be set once per rendering.
        renderer->setRenderCallback([=](const ImageData& imageData) {
            // Save the rendered result to the specified file-path 
            ResourceManager::writeImage(filePath, imageData);
        });

        float x = cos(Mathf::deg2Rad(degree)) * radius;
        float z = sin(Mathf::deg2Rad(degree)) * radius;
        degree += (360 / numCycles);

        // Update objects
        cam->getTransform().position = pointOfInterest + Point3f(x, 10, z);
        cam->getTransform().lookAt(pointOfInterest);

        i++;
        if (i == numCycles)
            renderingFinished = true;

#endif // !USE_WINDOW
    }

};


void Application::run()
{
    // Just uncomment this line to remove the Debug-Menu
    DebugMenu* dm = new DebugMenu(&renderer, FONT_GET("DebugFont", 24));

    SceneManager::switchScene(new MyScene());

#if !USE_WINDOW
    while (!renderingFinished)
    {
        // delta value is irrelevant in this case
        renderer.update(0);
        renderer.draw();
    }
#else // Render into a window and observe the scene (FPS Camera script is attached to the camera). 
    // #define for rendering a file instead is in HEADER-File
    while (window.update() && !Input::getKeyDown(KeyCodes::ESCAPE))
    {
        TimeManager::update();
        float delta = static_cast<float>(Time::getDeltaSeconds());
        InputManager::update(delta);
        renderer.update(delta);
        renderer.draw();
    }
#endif
}