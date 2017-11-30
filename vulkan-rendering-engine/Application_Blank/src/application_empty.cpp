#include "application.h"

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


class TestScene : public Scene
{
public:
    TestScene() : Scene("TestScene") {}
    ~TestScene() {}

    void init(RenderingEngine* renderer) override
    {
        Camera* cam = new Camera(Transform(Point3f(0, 0, 20)));
        cam->addComponent(new CMoveCamera(70, 3, 5, ECameraMode::MAYA));

        renderer->setCamera(cam);
    }
};


void Application::startLoop()
{
    Input::attachFunc(KeyCodes::C, [&] {renderer.toggleCulling(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::G, [&] {renderer.toggleGUI(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::V, [&] {renderer.toggleVSync(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::Z, [&] {renderer.toggleShadows(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::P, [&] {renderer.togglePostProcessing(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::F, [&] {renderer.toggleBoundingBoxes(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::H, [&] { ShaderManager::get("FXAA")->toggleActive(); }, Input::KEY_PRESSED);
    Input::attachFunc(KeyCodes::H, [&] { renderer.setRenderingMode(ERenderingMode::WIREFRAME); }, Input::KEY_PRESSED);

    // Change Window-Title text every second (1000ms)
    Time::setInterval([&] {
        //std::cout << "DELTA: " << Time::getDelta() / (float)Time::MILLISECOND << " ms" << std::endl;
        std::string windowTitle = "FPS: " + std::to_string(Time::getFPS()) + " (" + std::to_string(1000.0f / Time::getFPS()) + " ms)";
        window.setWindowText(windowTitle.c_str()); }
    , 1000);

    DebugMenu* dm = new DebugMenu(&renderer, TextureManager::getFont("DebugFont", 24));

    SceneManager::switchScene(new TestScene());

    // Render-Loop
    while (window.update() && !Input::getKeyDown(KeyCodes::ESCAPE))
    {
        TimeManager::update();
        update(static_cast<float>(Time::getDeltaSeconds()));
        renderer.draw();
    }

}

void Application::update(float delta)
{
    InputManager::update(delta);
    renderer.update(delta);
}