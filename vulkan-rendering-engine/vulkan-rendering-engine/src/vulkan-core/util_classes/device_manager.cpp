#include "device_manager.h"

#include "vulkan-core/vkTools/vk_tools.h"
#include "logger/logger.h"

#include <assert.h>

namespace Pyro
{
    #define LOG_GPUS 1
    #define PRINT_QUEUE_FAMILIES 0

    //---------------------------------------------------------------------------
    //  Public Members
    //---------------------------------------------------------------------------

    void DeviceManager::getGraphicQueue(uint32_t queueIndex, VkQueue *pQueue) const
    {
        assert(queueIndex <= selectedGPU.queueFamilyProperties[queueFamilyGraphicsIndex].queueCount);
        vkGetDeviceQueue(logicalDevice, queueFamilyGraphicsIndex, queueIndex, pQueue);
    }

    void DeviceManager::getPresentingQueue(uint32_t queueIndex, VkQueue *pQueue) const
    {
        assert(queueIndex <= selectedGPU.queueFamilyProperties[queueFamilyPresentingIndex].queueCount);
        vkGetDeviceQueue(logicalDevice, queueFamilyPresentingIndex, queueIndex, pQueue);
    }
    
    const GPU& DeviceManager::getGPU(uint32_t index) const
    {
        assert(index < gpus.size());
        return gpus[index];
    }

    // Initialize everything 
    void DeviceManager::init(VkInstance instance, const std::vector<const char*>& deviceExtensionNames)
    {
        // Save device-extension names
        this->deviceExtensionNames = deviceExtensionNames;

        // Search for GPU's on the system
        uint32_t numGPUs;
        vkEnumeratePhysicalDevices(instance, &numGPUs, NULL);

        if (numGPUs <= 0)
            Logger::Log("Did not find any GPU on your system.", LogType::LOGTYPE_ERROR);

        // Get all Physical Devices from Vulkan
        std::vector<VkPhysicalDevice> physicalDevices(numGPUs);
        VkResult res = vkEnumeratePhysicalDevices(instance, &numGPUs, physicalDevices.data());
        assert(res == VK_SUCCESS);

        // Enumerate over all GPU's and get their properties
        for (uint32_t i = 0; i < numGPUs; i++)
        {
            GPU gpu;
            gpu.gpu = physicalDevices[i];

            // get all queue familys from the GPU -> we are looking for a graphics AND
            // presenting family queue but first we need a surface then we can check 
            // whether a family supports both and create a logical device 
            vkGetPhysicalDeviceQueueFamilyProperties(gpu.gpu, &gpu.queueFamilyCount, nullptr);
            gpu.queueFamilyProperties.resize(gpu.queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(gpu.gpu, &gpu.queueFamilyCount, gpu.queueFamilyProperties.data());

            // get properties and features from the GPU
            vkGetPhysicalDeviceProperties(gpu.gpu, &gpu.properties);
            vkGetPhysicalDeviceMemoryProperties(gpu.gpu, &gpu.memoryProperties);
            vkGetPhysicalDeviceFeatures(gpu.gpu, &gpu.supportedFeatures);

            // Save it
            gpus.push_back(std::move(gpu));
        }

        // Find a suitable GPU and check device-layers & extensions
        selectGPU();
        checkDeviceLayersAndExtensions();
        initFeatures();
    }

    // Find suitable queue-familes which supports rendering and presenting to the surface
    void DeviceManager::selectQueueFamilies(const VkSurfaceKHR& surface)
    {
        // Search for a queue family which supports graphics and a queue family which supports presenting to the surface
        // Iterate over each queue to learn whether it supports presenting
        std::vector<VkBool32> supportsPresent(selectedGPU.queueFamilyCount);
        for (uint32_t i = 0; i < selectedGPU.queueFamilyCount; i++)
            vkGetPhysicalDeviceSurfaceSupportKHR(selectedGPU.gpu, i, surface, &supportsPresent[i]);

        // Search for a graphics queue 
        uint32_t graphicsQueueNodeIndex = UINT32_MAX;
        for (uint32_t i = 0; i < selectedGPU.queueFamilyCount; i++) {
            if ((selectedGPU.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                graphicsQueueNodeIndex = i;
                break;
            }
        }
        // Search for a presentation queue
        uint32_t presentingQueueNodeIndex = UINT32_MAX;
        for (uint32_t i = 0; i < selectedGPU.queueFamilyCount; i++) {
            if (supportsPresent[i] == VK_TRUE) {
                presentingQueueNodeIndex = i;
                break;
            }
        }

        // Check if valid queue-families were found
        if (graphicsQueueNodeIndex == UINT32_MAX || presentingQueueNodeIndex == UINT32_MAX)
            Logger::Log("Could not find a graphics and/or present queue family!", LogType::LOGTYPE_ERROR);

        queueFamilyGraphicsIndex = graphicsQueueNodeIndex;
        queueFamilyPresentingIndex = presentingQueueNodeIndex;
    }

    // Find suitable queue-familes which supports rendering
    void DeviceManager::selectQueueFamilies()
    {
        // Search for a graphics queue 
        uint32_t graphicsQueueNodeIndex = UINT32_MAX;
        for (uint32_t i = 0; i < selectedGPU.queueFamilyCount; i++) {
            if ((selectedGPU.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                graphicsQueueNodeIndex = i;
                break;
            }
        }

        // Check if a valid index was found
        if (graphicsQueueNodeIndex == UINT32_MAX)
            Logger::Log("Could not find a graphics queue family!", LogType::LOGTYPE_ERROR);

        queueFamilyGraphicsIndex = graphicsQueueNodeIndex;
    }



    //Create the logical device from the chosen GPU in "enumeratePhysicalDevices"
    VkDevice DeviceManager::createDevice(bool hasWindow)
    {
        // Holds informations about which queues from which families we want to create within a logical device
        std::vector<VkDeviceQueueCreateInfo> queueInfos;

        // We always need a queue for graphical operations
        float queuePriorities = 1.0f;
        VkDeviceQueueCreateInfo graphicQueueInfo;
        graphicQueueInfo.sType             = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicQueueInfo.queueFamilyIndex  = queueFamilyGraphicsIndex;
        graphicQueueInfo.pNext             = nullptr;
        graphicQueueInfo.flags             = 0;
        graphicQueueInfo.queueCount        = 1;
        graphicQueueInfo.pQueuePriorities  = &queuePriorities;

        queueInfos.push_back(graphicQueueInfo);

        if (hasWindow)
        {
            // Check if both graphics + presenting queue families are the same
            // If not we need a separate queue from that family
            if (queueFamilyGraphicsIndex != queueFamilyPresentingIndex)
            {
                //We need at least one queue with that family index for presenting the rendered images to the surface
                float qp[1] = { 1.0f };
                queueInfos[1].sType             = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueInfos[1].queueFamilyIndex  = queueFamilyPresentingIndex;
                queueInfos[1].pNext             = nullptr;
                queueInfos[1].flags             = 0;
                queueInfos[1].queueCount        = 1;
                queueInfos[1].pQueuePriorities  = qp;
            }
        }

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.pNext                    = nullptr;
        deviceInfo.queueCreateInfoCount     = static_cast<uint32_t>(queueInfos.size());
        deviceInfo.pQueueCreateInfos        = queueInfos.data();
        deviceInfo.enabledExtensionCount    = static_cast<uint32_t>(deviceExtensionNames.size());
        deviceInfo.ppEnabledExtensionNames  = deviceExtensionNames.data();
        deviceInfo.pEnabledFeatures         = &enabledFeatures;

        VkResult res = vkCreateDevice(selectedGPU.gpu, &deviceInfo, nullptr, &logicalDevice);
        assert(res == VK_SUCCESS);

        return logicalDevice;
    }


    //---------------------------------------------------------------------------
    //  Private Members
    //---------------------------------------------------------------------------

    // Selects the best GPU
    // Currently: Selects the first found discrete-GPU
    void DeviceManager::selectGPU()
    {
#if LOG_GPUS
        logGPUs();
#endif
        bool foundDiscreteGPU = false;

        // Iterate over properties and find a suitable GPU
        for (const auto& gpu : gpus)
        {
            // Look for a discrete-GPU
            if (gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                selectedGPU = gpu;
                foundDiscreteGPU = true;
                break;
            }
        }

        if (!foundDiscreteGPU)
        {
            for (const auto& gpu : gpus)
            {
                selectedGPU = gpu;
                break;
            }
        }

        Logger::Log("Selected GPU: " + std::string(selectedGPU.properties.deviceName), Pyro::LOGTYPE_INFO);
    }

    //Check if enabled Device Layers & Extensions are valid for a given gpu
    void DeviceManager::checkDeviceLayersAndExtensions()
    {
        const VkPhysicalDevice& gpu = selectedGPU.gpu;

        /* enumerate device layer properties */
        uint32_t layerCount;
        vkEnumerateDeviceLayerProperties(gpu, &layerCount, nullptr);
        std::vector<VkLayerProperties> layerProperties(layerCount);
        vkEnumerateDeviceLayerProperties(gpu, &layerCount, layerProperties.data());

        /* enumerate device extension properties */
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensionProperties(extensionCount);
        vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, extensionProperties.data());

        vkTools::checkEnabledExtensions(deviceExtensionNames, extensionProperties);
    }
    
    //Check if features are present and enable them
    void DeviceManager::initFeatures()
    {
        if (selectedGPU.supportedFeatures.fillModeNonSolid)
            enabledFeatures.fillModeNonSolid = VK_TRUE; // Used for Wireframe-Rendering
        else
            Logger::Log("Selected GPU does not support Wireframe Rendering.", LOGTYPE_WARNING);

        enabledFeatures.textureCompressionBC = VK_TRUE;
        enabledFeatures.samplerAnisotropy = VK_TRUE;
    }


    std::string getDeviceTypeAsString(const VkPhysicalDeviceType& type)
    {
        std::string name = "";
        switch (type)
        {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            name = "VK_PHYSICAL_DEVICE_TYPE_OTHER";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            name = "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
            break;
        case  VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            name = "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            name = "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            name = "VK_PHYSICAL_DEVICE_TYPE_CPU";
            break;
        }
        return name;
    }

    // Log important properties from all GPUs
    void DeviceManager::logGPUs()
    {
        std::string logText = "Print out important properties for all GPU's in the system:\n";
        logText += "Found " + std::to_string(gpus.size()) + " GPU in your system.\n";

        for (uint32_t i = 0; i < gpus.size(); i++)
        {
            const GPU& gpu = gpus[i];

            logText += "<<<< GPU #" + std::to_string(i) + ": " + std::string(gpu.properties.deviceName) + " >>>>\n";

            // GPU-Type
            logText += "GPU-Type: " + getDeviceTypeAsString(gpu.properties.deviceType) + "\n";

            uint32_t apiVers = gpu.properties.apiVersion;
            logText += "Vulkan API-Version: "  + std::to_string(VK_VERSION_MAJOR(apiVers)) + "." + 
                                                 std::to_string(VK_VERSION_MINOR(apiVers)) + "." + 
                                                 std::to_string(VK_VERSION_PATCH(apiVers)) + "\n";

#if PRINT_QUEUE_FAMILIES
            logText += "Amount of Queue Families: " + std::to_string(gpu.queueFamilyCount) + "\n";
            for (uint32_t j = 0; j < gpu.queueFamilyProperties.size(); j++)
            {
                const VkQueueFamilyProperties& qProps = gpu.queueFamilyProperties[j];
                logText += "Queue Family #" + std::to_string(j) + ": ";
                logText += "Queue Count: " + std::to_string(qProps.queueCount);
                logText += " Queue Flags: " + std::to_string(qProps.queueFlags) + "\n";
            }
#endif

            // More to come...
        }

        Logger::Log(logText, Pyro::LOGTYPE_INFO);
    }


}


