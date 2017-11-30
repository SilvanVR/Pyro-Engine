#include "vulkan_mesh_resource.h"

#include "vulkan-core/vulkan_base.h"

namespace Pyro
{


    // Load mesh data into gpu-memory and initialize the vulkan vertex- & indexBuffer
    VulkanMeshResource::VulkanMeshResource(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        uint32_t vertexBufferSize = static_cast<uint32_t>(vertices.size()) * sizeof(Vertex);
        uint32_t indexBufferSize = static_cast<uint32_t>(indices.size()) * sizeof(uint32_t);

        // Create host-mappable buffer for vertex-data
        VulkanBuffer stagingBufferVertices(VulkanBase::getDevice(), vertexBufferSize, 
                                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        // Copy vertex-data into the staging-buffer via memcpy
        stagingBufferVertices.copyInto(vertices.data(), vertexBufferSize);

        // Destination device local buffer
        vertexBuffer = std::unique_ptr<VulkanVertexBuffer>(new VulkanVertexBuffer(
                                                     VulkanBase::getDevice(), vertexBufferSize,
                                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
                                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

        // Create host-mappable buffer for index-data
        VulkanBuffer stagingBufferIndices(VulkanBase::getDevice(), indexBufferSize,
                                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        // Copy index-data into the staging-buffer via memcpy
        stagingBufferIndices.copyInto(indices.data(), indexBufferSize);

        // Destination device local buffer
        indexBuffer = std::unique_ptr<VulkanIndexBuffer>(new VulkanIndexBuffer(
                                       VulkanBase::getDevice(), indexBufferSize,
                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

        // Put buffer region copies into command buffer
        auto cmd = VulkanBase::getCommandPool()->allocate();
        cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        // Vertex buffer
        cmd->copyBuffer(stagingBufferVertices, *vertexBuffer, vertexBufferSize);

        // Index buffer
        cmd->copyBuffer(stagingBufferIndices, *indexBuffer, indexBufferSize);

        // Submit commands and wait for finishing
        cmd->endSubmitAndWaitForFence(VulkanBase::getDevice(), VulkanBase::getGraphicQueue());
    }



}



