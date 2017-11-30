#include "mesh.h"

#include "vulkan-core/data/material/texture/texture.h"
#include "vulkan-core/data/vulkan_mesh_resource.h"
#include "vulkan-core/data/material/material.h"


namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Mesh::Mesh(const std::string& _filePath)
        : FileResourceObject(_filePath), meshResource(nullptr)
    {
    }

    // Build a mesh from given vertices and indices
    Mesh::Mesh(const std::vector<Vertex>& vb, const std::vector<uint32_t>& _indices)
        : FileResourceObject("", "INTERNAL MESH"), vertices(vb), indices(_indices), meshResource(nullptr)
    {
        uploadDataToGPU();
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Mesh::~Mesh()
    {
        for(auto subMesh : subMeshes)
            delete subMesh;

        delete meshResource;
    }

    //---------------------------------------------------------------------------
    //  Mesh - Public Methods
    //---------------------------------------------------------------------------

    // Return a material from this mesh which was loaded during mesh-loading
    MaterialPtr Mesh::getMaterial(uint32_t index)
    {
        // Check if a material exists for the given index 
        // If this fails somehow check if the first material is named "DefaultMaterial", which is not allowed
        assert(hasMaterials() && materials.count(index) != 0);

        if (materials.count(index) == 0)
        {
            Logger::Log("Mesh::getMaterial(index): Given index '" + std::to_string(index) + 
                        "' does not exist in material-map", LOGTYPE_ERROR);
        }
        return materials[index]; 
    }

    // Bind this mesh (index & vertex-buffer) to the given cmd
    void Mesh::bind(VkCommandBuffer cmd)
    {
        // Bind vertices
        meshResource->getVertexBuffer()->bind(cmd, VERTEX_BUFFER_BIND_ID);

        // Bind indices
        meshResource->getIndexBuffer()->bind(cmd, 0, VK_INDEX_TYPE_UINT32);
    }

    // Record command for drawing this mesh into the given cmd
    void Mesh::draw(VkCommandBuffer cmd)
    {
        // Draw all submeshes
        for (const auto& subMesh : subMeshes)
            vkCmdDrawIndexed(cmd, subMesh->numIndices, 1, subMesh->startIndex, subMesh->startVertIndex, 0);
    }

    //---------------------------------------------------------------------------
    //  Mesh - Private Methods
    //---------------------------------------------------------------------------

    void Mesh::uploadDataToGPU()
    {
        assert(vertices.size() != 0 && indices.size() != 0 && meshResource == nullptr);
        meshResource = new VulkanMeshResource(vertices, indices);
    }

    //---------------------------------------------------------------------------
    //  SubMesh - Public Methods
    //---------------------------------------------------------------------------

    // Bind this mesh (index & vertex-buffer) to the given cmd
    void SubMesh::bind(VkCommandBuffer cmd)
    {
        parent->bind(cmd);
    }

    // Record command for drawing this mesh into the given cmd
    void SubMesh::draw(VkCommandBuffer cmd)
    {
        // Draw the submesh
        vkCmdDrawIndexed(cmd, numIndices, 1, startIndex, startVertIndex, 0);
    }

    // Return the material used by this submesh
    MaterialPtr SubMesh::getMaterial() 
    { 
        return parent->getMaterial(materialIndex); 
    }
}


