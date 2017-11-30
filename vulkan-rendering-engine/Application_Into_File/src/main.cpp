#include "application_into_file.h"

#if defined(NDEBUG) && defined(_WIN32)

    // No Console Window popping up. Directly call the windows main function.
    int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
    {
        Pyro::VFS::mount("models", "../vulkan-rendering-engine/res/models");
        Pyro::VFS::mount("textures", "../vulkan-rendering-engine/res/textures");
        Pyro::VFS::mount("fonts", "../vulkan-rendering-engine/res/fonts");
        Pyro::VFS::mount("shaders", "../vulkan-rendering-engine/res/shaders");

        Application app(800, 600);
        return 0;
    }

#else

    int main()
    {
        // Adapt the paths of the resource folders if necessary
        Pyro::VFS::mount("models", "../vulkan-rendering-engine/res/models");
        Pyro::VFS::mount("textures", "../vulkan-rendering-engine/res/textures");
        Pyro::VFS::mount("fonts", "../vulkan-rendering-engine/res/fonts");
        Pyro::VFS::mount("shaders", "../vulkan-rendering-engine/res/shaders");

        Application app(800, 600);
        return 0;
    }


#endif




