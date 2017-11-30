#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "vulkan-core/rendering_engine_interface.hpp"
#include "vulkan-core/window/window.h"

//---------------------------------------------------------------------------
//  Application class
//---------------------------------------------------------------------------

class Application
{
public:
    Pyro::Window window;
    Pyro::RenderingEngine renderer;

    Application();
    Application(int width, int height);
    ~Application() {}

    Pyro::RenderingEngine& getRenderer() { return renderer; }
    Pyro::Window& getWindow() { return window; }

    void startLoop();
    void update(float delta);
private:
    bool isRunning = true;
};


#endif



