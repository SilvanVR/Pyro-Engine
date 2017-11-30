#ifndef PLATFORM_H_
#define PLATFORM_H_

// Engine Namespace-name
#define ENGINE_STRING "Pyro"


#ifdef _WIN32

    #define NOMINMAX 
    // For MessageBox
    #include <Windows.h>

#endif // _WIN32



#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#endif // !PLATFORM_H_
