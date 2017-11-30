#ifndef VULKAN_MESH_RESOURCE
#define VULKAN_MESH_RESOURCE

#include "vulkan_resource.hpp"


namespace Pyro
{

    //---------------------------------------------------------------------------
    //  VulkanResource Class
    //---------------------------------------------------------------------------

    class VulkanMeshResource : public VulkanResource
    {

    public:
        // Load the texture data into gpu memory and initialize the VkDescriptorImageInfo struct for use in a descriptor set
        VulkanMeshResource(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        ~VulkanMeshResource() {}

        const std::unique_ptr<VulkanVertexBuffer>& getVertexBuffer() const { return vertexBuffer; }
        const std::unique_ptr<VulkanIndexBuffer>& getIndexBuffer() const { return indexBuffer; }

    private:
        //forbid copy and copy assignment
        VulkanMeshResource(const VulkanMeshResource& vulkanMeshResource) = delete;
        VulkanMeshResource& operator=(const VulkanMeshResource& vulkanMeshResource) = delete;

        // Vulkan Resources
        std::unique_ptr<VulkanVertexBuffer> vertexBuffer;
        std::unique_ptr<VulkanIndexBuffer> indexBuffer;
    };

}

#endif // !VULKAN_MESH_RESOURCE

