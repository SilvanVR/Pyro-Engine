/*
*  VulkanBase-Class header file.
*  Superclass of the Renderer. Try to abstracts the whole vulkan initilization from the Renderer itself.
*
*  Date:    23.04.2016
*  Author:  Silvan Hau
*/

#ifndef VULKAN_BASE_H_
#define VULKAN_BASE_H_

#include "build_options.h"

#include "cmd_pool_and_buffers/cmd_pool.h"
#include "util_classes/device_manager.h"
#include "window/window.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define GBUFFER_LIGHT_ACC_FORMAT        VK_FORMAT_R16G16B16A16_SFLOAT
    #define GBUFFER_NORMAL_FORMAT           VK_FORMAT_R16G16B16A16_SFLOAT
    #define GBUFFER_DEPTH_FORMAT            VK_FORMAT_D32_SFLOAT

    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class MappedValues;
    class Framebuffer;
    class Renderpass;
    class VMM;

    //---------------------------------------------------------------------------
    //  Structs
    //---------------------------------------------------------------------------

    // This struct contains necessary objects needed for rendering objects for one frame
    // It cant be reused until the work on the command-buffers has completed, thats why we have a fence here.
    // We use more than one of these structs, to use others ones while pending execution of the others
    struct FrameData {
        VulkanFence*                    fence;              // signaled when this struct is ready for reuse (blitCmd has finished execution)

        SCommandBuffer                  primaryCmd;         // Primary comand-buffer used for rendering the G-Buffer

        SCommandBuffer                  blitCmd;            // Command buffer used for copy the rendered offscreen result into the swapchain image

        VulkanSemaphore*                presentCompleteSem; // Semaphore signaled when presenting the image has completed
        
        Framebuffer*                    mrtFramebuffer;     // Deferred Rendering framebuffer (G-Buffer)
        Framebuffer*                    lightAccFramebuffer;// Target-Framebuffer for lighting
        Framebuffer*                    forwardFramebuffer; // Target-Framebuffer for forward rendering
    };

    //---------------------------------------------------------------------------
    //  VulkanBase class
    //---------------------------------------------------------------------------

    class VulkanBase
    {
    protected:
        struct Settings
        {
            bool enableValidation; // depends on debug/release mode
            bool vsync                  = false;
            bool cull                   = true;
            bool renderShadows          = false;
            bool renderGUI              = true;
            bool doPostProcessing       = true;
        } settings;
        
    public:
        // Initializes Vulkan with creating a swapchain from the given window in which the engine can render.
        VulkanBase(Window* window);

        // Initializes Vulkan without a window.
        VulkanBase(const Vec2ui& resolution);
        virtual ~VulkanBase();
        
        // Static Getters
        static VkDevice             getDevice()         { return INSTANCE->device0; }
        static VkQueue              getGraphicQueue()   { return INSTANCE->graphicQueue; }
        static CommandPool*         getCommandPool()    { return INSTANCE->commandPool; }
        static Renderpass*          getRenderpass()     { return INSTANCE->clearRenderpass; }
        static Renderpass*          getLightRenderpass(){ return INSTANCE->loadRenderpassNoDepth; }
        static Renderpass*          getMRTRenderpass()  { return INSTANCE->mrtRenderpass; }
        static Settings&            getSettings()       { return INSTANCE->settings; }
        static const GPU&           getGPU()            { return INSTANCE->deviceManager.getMainGPU(); }
        static int                  numFrameDatas()     { return INSTANCE->numFrameResources; }
        static int                  getFrameDataIndex() { return INSTANCE->frameDataIndex; }
        static const uint32_t&      getFinalWidth()     { if (INSTANCE->hasWindow()){ return Window::getWidth();}else{return INSTANCE->outputResolution.x();}  }
        static const uint32_t&      getFinalHeight()    { if (INSTANCE->hasWindow()){ return Window::getHeight();}else{return INSTANCE->outputResolution.y();} }

        // Toggle some settings
        void toggleVSync()                              { settings.vsync = !settings.vsync; }
        void toggleCulling()                            { settings.cull = !settings.cull; }
        void togglePostProcessing()                     { settings.doPostProcessing = !settings.doPostProcessing; }
        void setPostProcessing(bool doPostProcessing)   { settings.doPostProcessing = doPostProcessing;}
        void setVSync(bool vsync)                       { settings.vsync = vsync; }

        // Set the resolution-mod. 3D render dimension is (ResMod*Width),(ResMod*Height). Used for Down- or Supersampling.
        void setResolutionMod(float val)                { resolutionModifier = val; this->onSizeChanged(); }
        float getResolutionModifier()                   { return resolutionModifier; }

        // Change the render-dimension IF NOT a window is used. The engine will render in this new resolution.
        // Note that the final 3D render resolution is resolution * resolutionModifier. (Can be set through setResolutionModifier())
        void setFinalResolution(const Vec2ui& newRes);

        // Check if the renderer uses a window in which to draw
        bool inline hasWindow() { return window != nullptr; }

        // Return the resolution in which the engine renders the 3D-Scene. Its (ResMod*WindowWidth),(ResMod*WindowHeight)
        // if the engine has an window otherwise its (ResMod*renderResolutionWidth),(ResMod*renderResolutionHeight).
        Vec2ui get3DRenderResolution() { return Vec2ui(get3DRenderWidth(), get3DRenderHeight()) ; }
        uint32_t get3DRenderWidth(){return static_cast<uint32_t>(getFinalWidth() * resolutionModifier); }
        uint32_t get3DRenderHeight() { return static_cast<uint32_t>(getFinalHeight() * resolutionModifier); }

    protected:
        // The vulkan instance
        VkInstance                  instance;

        // An abstraction of an os-independent object in which the renderer will draw
        Window*                     window;
    
        // Handles the creation of an logical device
        DeviceManager               deviceManager;

        // A logical device which is created by the device-manager
        VkDevice                    device0;

        // Queues
        VkQueue                     graphicQueue;           // Actual graphic queues 
        VkQueue                     presentingQueue;        // Will be most likely the same as graphicsQueues[0]

        // Renderpasses
        Renderpass*                 clearRenderpass;        // Renderpass with color+depth attachment. Clears both
        Renderpass*                 mrtRenderpass;          // Renderpass with X-color attachments and a depth-attachment.
        //Renderpass*                 loadMrtRenderpass;      // Renderpass with X-color attachments and a depth-attachment.
        Renderpass*                 loadRenderpass;         // Renderpass with color+depth attachment. Both will be loaded.
        Renderpass*                 loadRenderpassNoDepth;  // Renderpass with light-acc color attachment.

        // Command Pool
        CommandPool*                commandPool;            // Command-Pool for primary command-buffers used by the main thread

        // This data is only used when the engine renders NOT in a window
        VkFormat                    finalColorFormat;       // Format from the albedo attachment
        Vec2ui                      outputResolution;       // The resolution in which the engine outputs the rendered image

        // Managers
        VMM*                        vmm;

        // Sampler for deferred lighting
        VulkanSampler*              gBufferSampler;

        // Frame-Resources
        const int numFrameResources = 3;
        std::vector<FrameData>      frameResources;
        uint32_t                    frameDataIndex = 0;
        uint32_t                    nextFrameDataIndex = 0;
        FrameData*                  currentFrameData;

        // Descriptor-Sets referencing the images from the G-Buffer
        MappedValues*               gBuffer;

        // Used to modify the resolution of the scene-framebuffer
        float resolutionModifier = 1.0f;

        // Layers, Extensions & Features
        std::vector<const char*>    instanceLayerNames;
        std::vector<const char*>    instanceExtensionNames;
        std::vector<const char*>    deviceExtensionNames;

        // Return the swapchains-surface format if a window is present otherwise it return a custom format
        const VkFormat& getSurfaceFormat() { if (hasWindow()) { return window->getSwapchain()->getSurfaceFormat().format; } else { return finalColorFormat; } }

        // Copy the given renderedImage into the appropriate 
        // swapchain-image and present it finally on screen
        void submitFrame(VulkanImage& renderedImage);

        // Called from the Window-Class as a callback if the window size has changed.
        virtual void onSizeChanged();

        // Recreate the offscreen framebuffers in which we render
        void recreateFramebuffer();

    private:
        // Instance used for static methods
        static VulkanBase* INSTANCE;

        void init();                                    //Initializes the whole class.
        void initLayersAndExtensions();                 //Enable Instance/Device Layers & Extensions
        void checkInstanceLayersAndExtensions();        //Check if enabled Instance Layers & Extensions are valid
        void createInstance();                          //Create the vulkan instance
        void initDeviceManager();                       //Initializes the device-manager
        void initWindow();                              //Create an VkSurface in the Window-Class (OS-specific) and search for an appropriate queue-family
        void findQueueFamilies();                       //Find appropriate queue families for rendering+presenting
        void createDevice();                            //Create the logical device
        void setupQueues();                             //Get the queue(s) from the logical device
        void initCommandPool();                         //Create a command pool for a primary cmd and for the secondary worker-cmds
        void initSwapchain();                           //Create the Swapchain in the window-class
        void initRenderpass();                          //Initialize the renderpass-class
        void initManager();                             //Initialize all necessary managers
        void initFrameResources();                      //Create frame-resource objects and initialize everything in it
        void initFramebuffer();                         //Create all framebuffers used for rendering the scene

    };

}






#endif // !VULKAN_BASE_H_



