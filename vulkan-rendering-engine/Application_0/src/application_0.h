#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "vulkan-core/rendering_engine_interface.hpp"
#include "vulkan-core/window/window.h"
#include "time/time_manager.h"

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

    void run();
    void update(float delta);
};


#endif



