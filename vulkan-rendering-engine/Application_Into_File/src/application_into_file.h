#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "vulkan-core/rendering_engine_interface.hpp"

// <<<<< INFORMATION >>>>>
// This project uses the engine build as a static-lib
// Build the engine once with the configuration Debug - StaticLib and/or Release - StaticLib
// then you can build and run this project :-) For questions email to silvan-hau@web.de

// Toggle this if you want to render into an window
#define USE_WINDOW 1

//---------------------------------------------------------------------------
//  Application class
//---------------------------------------------------------------------------

class Application
{
public:
#if USE_WINDOW
    Pyro::Window window;
#endif
    Pyro::RenderingEngine renderer;

    Application(int width, int height);
    ~Application() {}

    void run();
};


#endif



