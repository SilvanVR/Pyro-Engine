#ifndef DEVICE_MANAGER_H_
#define DEVICE_MANAGER_H_

#include "build_options.h"
#include "device.h"


namespace Pyro
{
    struct GPU
    {
        VkPhysicalDevice                        gpu;

        uint32_t                                queueFamilyCount;
        std::vector<VkQueueFamilyProperties>    queueFamilyProperties;
        VkPhysicalDeviceProperties              properties;
        VkPhysicalDeviceMemoryProperties        memoryProperties;
        VkPhysicalDeviceFeatures                supportedFeatures;
    };
    
    //---------------------------------------------------------------------------
    //  DeviceManager class
    //---------------------------------------------------------------------------

    class DeviceManager
    {

    public:
        DeviceManager() {}
        ~DeviceManager() {}

        // Initialize everything 
        void                init(VkInstance instance, const std::vector<const char*>& deviceExtensionNames);

        // Find suitable queue-familes which supports rendering and presenting to the surface
        void                selectQueueFamilies(const VkSurfaceKHR& surface);

        // Find suitable queue-familes which supports rendering
        void                selectQueueFamilies();

        // Creates the logical device with the requested amount of queues and enabled features
        VkDevice            createDevice(bool hasWindow);

        // Some Getter's
        void                getGraphicQueue(uint32_t queueIndex, VkQueue *pQueue) const;
        void                getPresentingQueue(uint32_t queueIndex, VkQueue *pQueue) const;
        uint32_t            getQueueFamilyGraphicsIndex() const { return queueFamilyGraphicsIndex; }
        uint32_t            getQueueFamilyPresentingIndex() const { return queueFamilyPresentingIndex; }
        VkDevice            getDevice() const { return logicalDevice; }
        VkPhysicalDevice    getPhysicalDevice() const { return selectedGPU.gpu; }
        const GPU&          getMainGPU() const { return selectedGPU; }
        const GPU&          getGPU(uint32_t index) const;

    private:
        // All GPUs in the system
        std::vector<GPU>            gpus;

        // Selected GPU which will be used for rendering
        GPU                         selectedGPU;

        // Logical Device created from the "selectedGPU"
        // Represents a GPU with enabled FEATURES and created Queues
        VkDevice                    logicalDevice;

        // Features which will be enabled on logical device creation
        VkPhysicalDeviceFeatures    enabledFeatures = {};
        
        // Extensions which should be enabled for the logical device
        std::vector<const char*>    deviceExtensionNames;

        // Queue-Families for graphics and presenting
        uint32_t                    queueFamilyGraphicsIndex;
        uint32_t                    queueFamilyPresentingIndex;

        // Check if features are present and enable them
        void initFeatures();
        
        // Select a GPU for rendering if there are more than two
        void selectGPU();

        // Check if device layers + extensions are present in vulkan
        void checkDeviceLayersAndExtensions();

        // Log important properties from all GPUs
        void                logGPUs();
    };



}


#endif // !DEVICE_MANAGER_H_
