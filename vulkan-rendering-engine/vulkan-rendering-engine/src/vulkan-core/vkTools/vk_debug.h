#ifndef VK_DEBUG_H_
#define VK_DEBUG_H_

#include "../../build_options.h"

namespace Pyro
{

    namespace vkDebug
    {

        //---------------------------------------------------------------------------
        //  Debug Callback Creation & Destruction
        //---------------------------------------------------------------------------
        void createDebugCallback(VkInstance instance);
        void destroyDebugCallback(VkInstance instance);


        //---------------------------------------------------------------------------
        //  Validation Debug Callback
        //---------------------------------------------------------------------------
        VKAPI_ATTR VkBool32 VKAPI_CALL
            VulkanDebugCallback(VkDebugReportFlagsEXT msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location,
                int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData);

    }

}


#endif // !VK_DEBUG_H_
