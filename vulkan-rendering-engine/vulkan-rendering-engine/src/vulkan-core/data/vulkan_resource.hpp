/*
*  VulkanResource-Class header file.
*  Serves as a base-class for all vulkan resources.
*
*  Date:    22.05.2016
*  Author:  Silvan Hau
*/


#ifndef VULKAN_RESOURCE
#define VULKAN_RESOURCE

#include "build_options.h"
#include "vulkan-core/util_classes/vulkan_buffer.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  VulkanResource Class
    //---------------------------------------------------------------------------

    class VulkanResource
    {

    public:
        VulkanResource() {};
        virtual ~VulkanResource() = 0 {};

    private:
        //forbid copy and copy assignment
        VulkanResource(const VulkanResource& vulkanResource) = delete;
        VulkanResource& operator=(const VulkanResource& vulkanResource) = delete;
    };

}


#endif // !VULKAN_RESOURCE
