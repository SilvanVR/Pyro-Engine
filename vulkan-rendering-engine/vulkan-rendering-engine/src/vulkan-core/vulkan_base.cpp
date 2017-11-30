#include "vulkan_base.h"

#include "memory_management/vulkan_memory_manager.h"
#include "pipelines/framebuffers/framebuffer.h"
#include "resource_manager/resource_manager.h"
#include "scene_graph/layers/layer_manager.h"
#include "pipelines/renderpass/renderpass.h"
#include "vkTools/vk_debug.h"
#include "vkTools/vk_tools.h"
#include "file_system/vfs.h"

#include <assert.h>
#include <functional>
#include <algorithm>

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define SHADER_SAMPLER_DEPTH_NAME       "SamplerDepth"
    #define SHADER_SAMPLER_NORMAL_NAME      "SamplerNormal"
    #define SHADER_SAMPLER_ALBEDO_NAME      "SamplerAlbedo"

    #define GBUFFER_SAMPLER_FILTER          VK_FILTER_NEAREST

    #define GBUFFER_LIGHT_ACC_INDEX         0
    #define GBUFFER_ALBEDO_INDEX            1
    #define GBUFFER_NORMAL_INDEX            2

    //---------------------------------------------------------------------------
    //  Static Members
    //---------------------------------------------------------------------------

    VulkanBase* VulkanBase::INSTANCE = nullptr;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    VulkanBase::VulkanBase(Window* window)
    {
        this->window = window;
        init();
    }

    // Initializes Vulkan without creating a surface.
    VulkanBase::VulkanBase(const Vec2ui& resolution)
        : outputResolution(resolution), finalColorFormat(VK_FORMAT_B8G8R8A8_UNORM)
    {
        this->window = nullptr;
        init();
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    VulkanBase::~VulkanBase()
    {
        VkResult res = vkDeviceWaitIdle(device0);
        assert(res == VK_SUCCESS);
        // Order is important here
        ResourceManager::destroy();
        LayerManager::destroy();
        for (auto& frameResource : frameResources)
        {
            delete frameResource.fence;
            delete frameResource.presentCompleteSem;
            delete frameResource.mrtFramebuffer;
            delete frameResource.lightAccFramebuffer;
            delete frameResource.forwardFramebuffer;
            frameResource.blitCmd.reset();
            frameResource.primaryCmd.reset();
        }
        delete gBuffer;
        delete vmm;
        delete commandPool;
        delete clearRenderpass;
        delete mrtRenderpass;
        delete loadRenderpass;
        delete loadRenderpassNoDepth;
        delete gBufferSampler;
        if(hasWindow()) window->destroy(instance);
        vkDestroyDevice(device0, nullptr);
        if (settings.enableValidation)
            vkDebug::destroyDebugCallback(instance);
        vkDestroyInstance(instance, nullptr);
    }

    //---------------------------------------------------------------------------
    //  Public Members
    //---------------------------------------------------------------------------

    void VulkanBase::setFinalResolution(const Vec2ui& newRes)
    {
        assert(!hasWindow());
        outputResolution = newRes;
        this->onSizeChanged();
    }

    //---------------------------------------------------------------------------
    //  Protected Members
    //---------------------------------------------------------------------------

    //Recreate all Framebuffers in the FrameData-structs
    void VulkanBase::recreateFramebuffer()
    {
        for (unsigned int i = 0; i < frameResources.size(); i++)
        {
            delete frameResources[i].mrtFramebuffer;
            delete frameResources[i].forwardFramebuffer;
            delete frameResources[i].lightAccFramebuffer;
        }

        initFramebuffer();
    }

    //---------------------------------------------------------------------------
    //  Private Members
    //---------------------------------------------------------------------------

    void VulkanBase::init()
    {
#ifdef NDEBUG
        settings.enableValidation = false;
#else
        settings.enableValidation = true;
#endif

        if (INSTANCE == nullptr)
            INSTANCE = this;
        else
            Logger::Log("ERROR in VulkanBase::VulkanBase(): Two VulkanBase Objects were created, which is not allowed.", LogType::LOGTYPE_ERROR);
        
        initLayersAndExtensions();
        checkInstanceLayersAndExtensions();
        createInstance();
        if (settings.enableValidation) vkDebug::createDebugCallback(instance);
        initDeviceManager();
        if(hasWindow())
            initWindow();
        findQueueFamilies();
        createDevice();
        setupQueues();
        initCommandPool();
        if(hasWindow())
            initSwapchain();
        initRenderpass();
        initManager();
        initFrameResources();
        initFramebuffer();
        Logger::Log("Finished initalizing all vulkan frame-related resources.", LOGTYPE_INFO);
    }

    //Enable Instance/Device Layers & Extensions
    void VulkanBase::initLayersAndExtensions()
    {
        /* Instance Layers & Extensions */
        if (settings.enableValidation)
        {
            instanceLayerNames.push_back("VK_LAYER_LUNARG_standard_validation");    /* Enable standard validation layers if validation is true */
            instanceExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);   /* Enable debug callback extension */
        }

        /* Enable surface extensions (platform-dependant) queried from GLWF */
        std::vector<const char*> surfaceExtensions = window->getRequiredInstanceExtensions();
        instanceExtensionNames.insert(instanceExtensionNames.end(), surfaceExtensions.begin(), surfaceExtensions.end());

        /* Device Extensions (device layers are deprecated) */
        deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);            /* Enable swapchain extension */
    }

    //Check if enabled Instance Layers & Extensions are valid
    void VulkanBase::checkInstanceLayersAndExtensions()
    {
        /* enumerate layer properties */
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> layerProperties(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

        /* enumerate extension properties */
        uint32_t extensionCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensionProperties(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());

        vkTools::checkEnabledLayers(instanceLayerNames, layerProperties);
        vkTools::checkEnabledExtensions(instanceExtensionNames, extensionProperties);
    }

    //Create the vulkan instance
    void VulkanBase::createInstance()
    {
        /* Required Vulkan Version */
        int major = 1;
        int minor = 0;
        Logger::Log("Required Vulkan Version: [" + std::to_string(major) + "." + std::to_string(minor) + "]");

        /* Application Information */
        VkApplicationInfo applicationInfo = {};
        applicationInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pNext               = nullptr;
        applicationInfo.pApplicationName    = VK_NULL_HANDLE;
        applicationInfo.applicationVersion  = 1;
        applicationInfo.pEngineName         = VK_NULL_HANDLE;
        applicationInfo.engineVersion       = 1;
        applicationInfo.apiVersion          = VK_MAKE_VERSION(major,minor,0); // Patch is irrelevant

        /* Instance Create Information */
        VkInstanceCreateInfo instanceInfo = {};
        instanceInfo.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pNext                      = nullptr;
        instanceInfo.flags                      = 0;
        instanceInfo.pApplicationInfo           = &applicationInfo;
        instanceInfo.enabledLayerCount          = static_cast<uint32_t>(instanceLayerNames.size());
        instanceInfo.ppEnabledLayerNames        = instanceLayerNames.data();
        instanceInfo.enabledExtensionCount      = static_cast<uint32_t>(instanceExtensionNames.size());
        instanceInfo.ppEnabledExtensionNames    = instanceExtensionNames.data();

        /* Create Vulkan Instance */
        VkResult res = vkCreateInstance(&instanceInfo, nullptr, &instance);

        if (res == VK_ERROR_INCOMPATIBLE_DRIVER)
            Logger::Log("Cant find an expected vulkan driver with version [" + std::to_string(major) + "." + std::to_string(minor) + "]", LogType::LOGTYPE_ERROR);
        else if (res)
            Logger::Log("Unknown error in creating a vulkan instance. Please contact the Assault Team!", LogType::LOGTYPE_ERROR);

    }

    //Initializes the device-manager
    void VulkanBase::initDeviceManager()
    {
        // depends on createInstance()
        deviceManager.init(instance, deviceExtensionNames);
    }

    //Create an VkSurface in the Window-Class (OS-specific) and search for an appropriate queue-family which supports graphics and presenting
    void VulkanBase::initWindow()
    {
        // depends on and initDeviceManager()
        // Initialize VkSurface in Window-Class and bind a callback function
        window->initSurface(instance, std::bind(&VulkanBase::onSizeChanged, this));
    }
    
    //Find appropriate queue families for rendering+presenting
    void VulkanBase::findQueueFamilies()
    {
        // depends on initWindow() or initDeviceManager()
        if(hasWindow())
            deviceManager.selectQueueFamilies(window->getVulkanSurface());
        else
            deviceManager.selectQueueFamilies();
    }

    //Create the logical device
    void VulkanBase::createDevice()
    {
        // depends on findQueueFamilies()
        device0 = deviceManager.createDevice(hasWindow());
    }

    //Get the queue(s) from the logical device (graphicsQueues[0] = presentingQueue if ..GraphicsIndex equal ..PresentingIndex)
    void VulkanBase::setupQueues()
    {
        // depends on createDevice()
        deviceManager.getGraphicQueue(0, &graphicQueue);
        if (hasWindow())
            deviceManager.getPresentingQueue(0, &presentingQueue);
    }

    //Create a command pool for a primary cmd and for the secondary worker-cmds
    void VulkanBase::initCommandPool()
    {
        // depends on createDevice()
        commandPool = new CommandPool(device0, deviceManager.getQueueFamilyGraphicsIndex(),
                                      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    }

    //Create the Swapchain in the window-class
    void VulkanBase::initSwapchain()
    {
        // depends on createDevice()
        window->initSwapchain(deviceManager);
    }

    //Initialize the renderpass-class
    void VulkanBase::initRenderpass()
    {
        VkAttachmentDescription normalAttachment, albedoAttachment, lightAccAttachment, depthAttachment;

        // Light-Accumulation attachment
        lightAccAttachment.flags          = 0;
        lightAccAttachment.format         = GBUFFER_LIGHT_ACC_FORMAT;
        lightAccAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        lightAccAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        lightAccAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        lightAccAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        lightAccAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        lightAccAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        lightAccAttachment.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Albedo attachment
        albedoAttachment.flags          = 0;
        albedoAttachment.format         = getSurfaceFormat();
        albedoAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        albedoAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        albedoAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        albedoAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        albedoAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        albedoAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        albedoAttachment.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Normal attachment
        normalAttachment.flags            = 0;
        normalAttachment.format           = GBUFFER_NORMAL_FORMAT;
        normalAttachment.samples          = VK_SAMPLE_COUNT_1_BIT;
        normalAttachment.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        normalAttachment.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        normalAttachment.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        normalAttachment.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        normalAttachment.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
        normalAttachment.finalLayout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Depth attachment
        depthAttachment.flags           = 0;
        depthAttachment.format          = GBUFFER_DEPTH_FORMAT;
        depthAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // IMPORTANT ON AMD - GPU's!!!!

        // Renderpass for gBuffer
        mrtRenderpass = new Renderpass(device0, { { lightAccAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
                                                  { albedoAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
                                                  { normalAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } },
                                                { depthAttachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });

        // Basic Renderpass
        clearRenderpass = new Renderpass(device0, { lightAccAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
                                                  { depthAttachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });

        //// Renderpass for gBuffer which loads the attachments instead of clearing it
        //lightAccAttachment.loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
        //lightAccAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        //loadMrtRenderpass = new Renderpass(device0, { { lightAccAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
        //                                              { albedoAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
        //                                              { normalAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } },
        //                                              { depthAttachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });

        // Renderpass for the forward rendering pass. It loads the color + depth attachment
        lightAccAttachment.loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
        lightAccAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        loadRenderpass = new Renderpass(device0, { lightAccAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
                                                 { depthAttachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });

        // Deferred renderpass for deferred lighting. Loads color-Attachment.
        loadRenderpassNoDepth = new Renderpass(device0, { lightAccAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
    }

    //Create frame-resource objects and initialize everything
    void VulkanBase::initFrameResources()
    {
        // Sampler for GBuffer
        gBufferSampler = new VulkanSampler(device0, GBUFFER_SAMPLER_FILTER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

        // Contains descriptor-sets for each frame-data
        gBuffer = new MappedValues("DescriptorSets#SamplerDepth");

        frameResources.resize(numFrameResources);
        for (unsigned int i = 0; i < frameResources.size(); i++)
        {
            // Create Fence
            frameResources[i].fence = new VulkanFence(device0, VK_FENCE_CREATE_SIGNALED_BIT);

            // Create Semaphores
            frameResources[i].presentCompleteSem = new VulkanSemaphore(device0);

            // Allocate primary command buffer
            frameResources[i].primaryCmd = commandPool->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

            // Allocate blit command buffer
            frameResources[i].blitCmd = commandPool->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        }
    }

    // Create all necessary managers
    void VulkanBase::initManager()
    {
        vmm = new VMM(this);

        // Set some default file-locations if they weren't set before
        VFS::mount("models", "res/models", false);
        VFS::mount("textures", "res/textures", false);
        VFS::mount("fonts", "res/fonts", false);
        VFS::mount("shaders", "res/shaders", false);
        VFS::mount("log", "res/logs", false);
        VFS::mount("scenes", "res/scenes", false);

#if NDEBUG
        Logger::setLogLevel(LOG_LEVEL_IMPORTANT);
#else
        //Logger::setLogLevel(LOG_LEVEL_ALL);
        Logger::setLogLevel(LOG_LEVEL_NOT_SO_IMPORTANT);
#endif
        LayerManager::init();
        ResourceManager::init();
    }

    // Create all framebuffers used for rendering the scene
    void VulkanBase::initFramebuffer()
    {
        VkImageUsageFlags colorUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VkImageUsageFlags depthUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        for (unsigned int i = 0; i < frameResources.size(); i++)
        {
            uint32_t width = std::max((int)(getFinalWidth() * resolutionModifier), 1);
            uint32_t height = std::max((int)(getFinalHeight() * resolutionModifier), 1);

            // The main deferred rendering framebuffer which creates and manages all buffers
            frameResources[i].mrtFramebuffer = new Framebuffer(device0, mrtRenderpass, width, height,
                                                              { colorUsage, colorUsage, colorUsage }, depthUsage);

            std::vector<std::shared_ptr<VulkanImageView>> imageViews = { frameResources[i].mrtFramebuffer->getColorView(GBUFFER_LIGHT_ACC_INDEX) };

            // Framebuffer with the light-acc as the color-target 
            frameResources[i].lightAccFramebuffer = new Framebuffer(device0, loadRenderpassNoDepth, width, height, imageViews);

            // Framebuffer with the light-acc as the color-target and the mrt-depth-buffer
            frameResources[i].forwardFramebuffer = new Framebuffer(device0, loadRenderpass, width, height,
                                                                   imageViews, frameResources[i].mrtFramebuffer->getDepthView());

            // Reference the G-Buffer in the g-buffer descriptor-set
            VkDescriptorImageInfo imageInfo;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.sampler     = gBufferSampler->get();

            imageInfo.imageView = frameResources[i].mrtFramebuffer->getDepthView()->get();
            gBuffer->setTexture(i, SHADER_SAMPLER_DEPTH_NAME, imageInfo);

            imageInfo.imageView = frameResources[i].mrtFramebuffer->getColorView(GBUFFER_ALBEDO_INDEX)->get();
            gBuffer->setTexture(i, SHADER_SAMPLER_ALBEDO_NAME, imageInfo);

            imageInfo.imageView = frameResources[i].mrtFramebuffer->getColorView(GBUFFER_NORMAL_INDEX)->get();
            gBuffer->setTexture(i, SHADER_SAMPLER_NORMAL_NAME, imageInfo);
        }
    }

    // Copy the given renderedImage into the appropriate 
    // swapchain-image and present it finally on screen
    void VulkanBase::submitFrame(VulkanImage& renderedImage)
    {
        Swapchain* swapchain = window->getSwapchain();
        FrameData& frameData = frameResources[frameDataIndex];

        // Next image in the swapchain used for presenting
        uint32_t nextImage;

        // Aquire next image. Present-Complete Semaphore gets signaled when presentation is complete.
        VkResult result = swapchain->aquireNextImageIndex(UINT64_MAX, frameData.presentCompleteSem, NULL, &nextImage);
        switch (result)
        {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            this->onSizeChanged();
            break;
        default:
            Logger::Log("Problem occurred during swap chain image acquisition!", LogType::LOGTYPE_ERROR);
        }

        // Copy the rendered image into the appropriate swapchain-image
        CommandBuffer& blitCmd = *frameData.blitCmd;

        blitCmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        {
            VkImageSubresourceRange subResourceRange = {};
            subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subResourceRange.layerCount = 1;
            subResourceRange.levelCount = 1;

            // Transition Layouts
            blitCmd.setImageLayout(renderedImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subResourceRange);
            blitCmd.setImageLayout(swapchain->getImage(nextImage), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subResourceRange);

            // Perform copy
            blitCmd.copyImage(renderedImage, swapchain->getImage(nextImage));

            // Retransition Layouts
            blitCmd.setImageLayout(renderedImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subResourceRange);
            blitCmd.setImageLayout(swapchain->getImage(nextImage), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, subResourceRange);
        }
        blitCmd.end();

        // Submit the copying command, wait until the image has been presented
        // This is the last submit before presenting so signal the fence in the frame-data struct
        blitCmd.submit(graphicQueue, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                       frameData.presentCompleteSem, nullptr, frameData.fence);

        // Submit swapchain-image to the presentation engine
        result = swapchain->queuePresent(presentingQueue, {}, nextImage);
        switch (result)
        {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            this->onSizeChanged();
            break;
        default:
            Logger::Log("Problem occurred during queue present!", LogType::LOGTYPE_ERROR);
        }

        // Recreate swapchain, renderpass and framebuffer if vsync has changed last frame
        {
            static bool vSyncLastFrame = false;
            if (vSyncLastFrame != settings.vsync)
            {
                vkDeviceWaitIdle(device0);
                window->getSwapchain()->recreate();
                onSizeChanged();
                vSyncLastFrame = settings.vsync;
            }
        }
    }

    //---------------------------------------------------------------------------
    //  OnSizeChanged Callback
    //---------------------------------------------------------------------------

    // Called from the window class as a callback if e.g. the window size has changed. 
    // Recreates the renderpass, swapchain, framebuffers.
    void VulkanBase::onSizeChanged()
    {
        vkDeviceWaitIdle(device0);

        //// Save the first renderpass and dont destroy it
        //if (initialRenderpass == nullptr)
        //    initialRenderpass = mrtRenderpass;
        //else
        //    delete mrtRenderpass;
        //
        //// Recreates the renderpass using the formats of the new swapchain (recreated in the window-class)
        //initRenderpass();
        //
        // Recreate framebuffer
        recreateFramebuffer();

        // UPDATE: 
        // If we dont delete the renderpass which the pipelines were created with we dont have to 
        // recreate all pipelines as long the new renderpass is compatible with the initial one.

        // Recreate all pipelines using the new renderPass
        //pipelineManager->recreatePipelines(renderpass->get());
    };

}
