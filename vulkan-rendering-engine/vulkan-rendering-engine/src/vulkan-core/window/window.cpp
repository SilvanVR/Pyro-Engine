#include "window.h"
#include "Input/input_manager.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  GLFW - Callbacks
    //---------------------------------------------------------------------------

    void error_callback(int error, const char* description);
    void window_size_callback(GLFWwindow* window, int width, int height);
    void cursor_position_callback(GLFWwindow* window, double xPos, double yPos);
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    Window* Window::INSTANCE;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Window::Window(int _width, int _height)
    {
        width  = _width;
        height = _height;

        // Set SINGLETON Instance - object
        if(INSTANCE == nullptr)
            INSTANCE = this;
        else
            return;

        if (!glfwInit()){  // Initialization failed
            return;
        }

        // Setup error callback
        glfwSetErrorCallback(error_callback);

        createWindow();
    }

    // Create a fullscreen window
    Window::Window()
    {
        // Set SINGLETON Instance - object
        if (INSTANCE == nullptr)
            INSTANCE = this;
        else
            return;

        if (!glfwInit()) {  // Initialization failed
            return;
        }

        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        width = mode->width;
        height = mode->height;

        // Setup error callback
        glfwSetErrorCallback(error_callback);

        Logger::Log("FULLSCREEN-Mode is used. May not work properly. (Window-Constructor without parameters)");

        createWindow(true);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Window::~Window() 
    {
        glfwTerminate();
    }

    //---------------------------------------------------------------------------
    //  Public Members
    //---------------------------------------------------------------------------

    void Window::setMouseCursorVisibiblity(bool visible)
    {
        glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    std::vector<const char*> Window::getRequiredInstanceExtensions()
    {
        unsigned int glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        std::vector<const char*> instanceExtensions;
        for (unsigned int i = 0; i < glfwExtensionCount; i++)
        {
            const char* glfwExtension = *glfwExtensions;
            instanceExtensions.push_back(glfwExtension);
            *(glfwExtensions++);
        }

        return instanceExtensions;
    }

    void Window::initSurface(const VkInstance& instance, const std::function<void()>& func)
    {
        this->rendererCallback = func;
        glfwCreateWindowSurface(instance, window, NULL, &surface);
    }

    void Window::initSwapchain(const DeviceManager& deviceManager)
    {
        swapchain = new Swapchain(deviceManager, this);
    }

    void Window::destroy(const VkInstance& instance)
    {
        delete swapchain;
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }

    bool Window::update()
    {
        glfwPollEvents();
        return !glfwWindowShouldClose(window);
    }

    void Window::setWindowText(const char* text)
    {
        glfwSetWindowTitle(window, text);
    }

    // Fullscreen windowed-mode without a task-bar
    //void Window::toggleFullScreenWindowed()
    //{
    //    fullscreen = !fullscreen;
    //    if(fullscreen)
    //        glfwWindowHint(GLFW_DECORATED, false);
    //    else
    //        glfwWindowHint(GLFW_DECORATED, true);
    //    createWindow();
    //}

    //---------------------------------------------------------------------------
    //  Private Members
    //---------------------------------------------------------------------------
    
    void Window::createWindow(bool fullscreen)
    {
        // Tell GLFW not to create an OpenGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        if(window != nullptr)
            glfwDestroyWindow(window);

        GLFWmonitor* monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;
        window = glfwCreateWindow(width, height, "Pyro Engine", monitor, nullptr);

        if (!window)
        {
            // window creation failed
            glfwTerminate();
            return;
        }

        // Setup window-size callback
        glfwSetWindowSizeCallback(window, window_size_callback);

        // Mouse callbacks
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetScrollCallback(window, scroll_callback);

        // Key callback
        glfwSetKeyCallback(window, key_callback);

        centerWindow();
    }

    // Center the window
    void Window::centerWindow()
    {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowPos(window, (mode->width / 2) - (width / 2), (mode->height / 2) - (height / 2));
    }

    //---------------------------------------------------------------------------
    //  Static Functions
    //---------------------------------------------------------------------------

    //Called if the window-size changes from os-specific code and notifies the renderer about it
    void Window::onSizeChanged(uint32_t newWidth, uint32_t newHeight)
    {
        if (INSTANCE->rendererCallback == nullptr || newWidth == 0 || newHeight == 0)
            return;
        INSTANCE->width = newWidth;
        INSTANCE->height = newHeight;

        // Recreate swapchain with new width and height
        INSTANCE->swapchain->recreate();

        INSTANCE->rendererCallback();
    }


    //---------------------------------------------------------------------------
    //  GLFW - Callbacks
    //---------------------------------------------------------------------------

    void error_callback(int error, const char* description)
    {
        fprintf(stderr, "Error: %s\n", description);
    }

    void window_size_callback(GLFWwindow* window, int width, int height)
    {
        Window::onSizeChanged((uint32_t)width, (uint32_t)height);
    }

    void cursor_position_callback(GLFWwindow* window, double xPos, double yPos)
    {
        InputManager::setMousePos((unsigned int)xPos, (unsigned int)yPos);
    }

    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        if (action == GLFW_PRESS)
            InputManager::setKeyDown(button);
        else if (action == GLFW_RELEASE)
            InputManager::setKeyUp(button);
    }

    void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
    {
        InputManager::setWheelDelta((short)yOffset);
    }

    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
            InputManager::setKeyDown(key);
        else if(action == GLFW_RELEASE)
            InputManager::setKeyUp(key);
    }



}