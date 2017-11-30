#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "vulkan-core/rendering_engine_interface.hpp"

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

private:
    void initDebugMenu();
};


#endif



