#ifndef WINDOW_H_
#define WINDOW_H_

#include "build_options.h"
#include "swapchain/swapchain.h"

#include <functional>

namespace Pyro
{
    class DeviceManager;

    class Window
    {
        friend class Swapchain; // Allow the swapchain to set the private width/height fields (e.g. when window size has changed)

        // Singleton-instance of this class
        static Window* INSTANCE;

    public:
        // Create a window with the requested size
        Window(int width, int height);
        // Create a fullscreen window
        Window();
        ~Window();

        // Called if the window-size changes and notifys the renderer (calls the rendererCallback)
        static void onSizeChanged(uint32_t newWidth, uint32_t newHeight);

        static const uint32_t& getWidth() { return INSTANCE->width; }
        static const uint32_t& getHeight() { return INSTANCE->height; }

        // Initialize the VkSurface object - called after initializing the Vulkan Instance from VulkanBase 
        // onWindowSizeChangedCallback will be called if the window size or sth like this has changed
        void initSurface(const VkInstance& instance, const std::function<void()>& onWindowSizeChangedCallback);

        // Initialize the Vulkan-Swapchain, which handles the presentation of the rendered-images
        void initSwapchain(const DeviceManager& deviceManager);

        // Destroy`s all vulkan related resources (surface and swapchain)
        void destroy(const VkInstance& instance);

        // Update window once per frame (return val indicates whether this Window should be closed)
        bool update();

        Swapchain*              getSwapchain() const { return swapchain; }
        const VkSurfaceKHR&     getVulkanSurface() const { return surface; }

        // Set the text in the windows header bar
        void setWindowText(const char* text);

        void setMouseCursorVisibiblity(bool visible);

        // Fullscreen windowed-mode without a task-bar
        //void toggleFullScreenWindowed();

        // Return the required Instance-Extensions for creating a Surface
        std::vector<const char*> getRequiredInstanceExtensions();

    private:
        uint32_t              width, height;                        // Width & Height of the Window
        VkSurfaceKHR          surface           = VK_NULL_HANDLE;   // Platform-independant surface from vulkan. Used by the swapchain.
        std::function<void()> rendererCallback  = nullptr;          // A Callback for the renderer to notify him when the window size has changed
        Swapchain*            swapchain         = nullptr;          // Custom class, which encapsulates the VkSwapchain and recreation-possibility
        GLFWwindow*           window            = nullptr;          // GLFW Window-Handle

        // Center the window
        void centerWindow();

        // Create the GLWF-Window (and destroys the old one if existing)
        void createWindow(bool fullscreen = false);
    };

}


#endif // !WINDOW_H_




