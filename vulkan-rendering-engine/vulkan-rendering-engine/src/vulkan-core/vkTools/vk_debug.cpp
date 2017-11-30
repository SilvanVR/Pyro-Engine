#include "vk_debug.h"

#include <assert.h>
#include <sstream>
#include <iostream>

namespace Pyro
{

    namespace vkDebug
    {

        //---------------------------------------------------------------------------
        //  Debug Callback Creation & Destruction
        //---------------------------------------------------------------------------

        /* Debug report callback */
        VkDebugReportCallbackEXT debugReportCallback;

        void createDebugCallback(VkInstance instance)
        {
            PFN_vkCreateDebugReportCallbackEXT dbgCreateDebugReportCallback;

            dbgCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
            if (!dbgCreateDebugReportCallback) {
                assert(0 && "GetInstanceProcAddr: Unable to find vkCreateDebugReportCallbackEXT function.");
                exit(1);
            }
            VkDebugReportCallbackCreateInfoEXT callbackInfo = {};
            callbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
            callbackInfo.pNext = NULL;
            callbackInfo.flags =
                //VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                VK_DEBUG_REPORT_WARNING_BIT_EXT |
                //VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                VK_DEBUG_REPORT_ERROR_BIT_EXT |
                //VK_DEBUG_REPORT_DEBUG_BIT_EXT | 
                0;
            callbackInfo.pfnCallback = VulkanDebugCallback;
            callbackInfo.pUserData = NULL;

            VkResult res = dbgCreateDebugReportCallback(instance, &callbackInfo, NULL, &debugReportCallback);

            if (res == VK_ERROR_OUT_OF_HOST_MEMORY) {
                assert(0 && "dbgCreateDebugReportCallback: out of host memory");
                exit(-1);
            }
            else if (res) {
                assert(0 && "dbgCreateDebugReportCallback: unknown failure");
                exit(-1);
            }
        }

        void destroyDebugCallback(VkInstance instance)
        {
            PFN_vkDestroyDebugReportCallbackEXT dbgDestroyDebugReportCallback;

            dbgDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
            if (!dbgDestroyDebugReportCallback) {
                assert(0 && "GetInstanceProcAddr: Unable to find vkDestroyDebugReportCallbackEXT function.");
                exit(1);
            }

            /* Clean up debug callback */
            dbgDestroyDebugReportCallback(instance, debugReportCallback, NULL);
        }


        //---------------------------------------------------------------------------
        //  Validation Debug Callback
        //---------------------------------------------------------------------------
        VKAPI_ATTR VkBool32 VKAPI_CALL
            VulkanDebugCallback(VkDebugReportFlagsEXT msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location,
                int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData)
        {
            std::ostringstream message;

            if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
                message << "ERROR: ";
            else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
                message << "WARNING: ";
            else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
                message << "PERFORMANCE WARNING: ";
            else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
                message << "INFO: ";
            else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
                message << "DEBUG: ";
            message << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;

        #ifdef _WIN32
            MessageBox(NULL, message.str().c_str(), "Vulkan Validation Error", MB_OK);
            if(msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
                __debugbreak();
        #else
            std::cout << message.str() << std::endl;
        #endif

            return false;
        }


    }

}