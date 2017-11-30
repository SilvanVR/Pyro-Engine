/*
*  Mesh class which describes a 3D-mesh with vertices and indices.
*
*  Date:    02.05.2016
*  Author:  Silvan Hau
*/

#ifndef MESH_H_
#define MESH_H_

#include "build_options.h"
#include "vulkan-core/resource_manager/file_resource_object.hpp"
#include "data_types.hpp"

namespace Pyro
{

    class VulkanMeshResource;
    class Material;
    class Texture;
    class SubMesh;

    // Dimension for a mesh to do view-frustum culling
    struct Dimension
    {
        Vec3f min           = Vec3f(FLT_MAX, FLT_MAX, FLT_MAX);
        Vec3f max           = Vec3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        Vec3f size          = Vec3f(0,0);
        float maxRadius     = 0.0f;

        // The local-position from the Sub-Mesh in compare to the parents-origin.
        // Its the original vertex-position minus the computed 3D-centroid for that submesh.
        Point3f localPosition = Point3f(0,0,0);

        // Union the given Dimension to this one, so this one has the max-values of both dimensions.
        void unionDimensions(const Dimension& other)
        {
            min = min.minVec(other.min);
            max = max.maxVec(other.max);
            maxRadius = maxRadius > other.maxRadius ? maxRadius : other.maxRadius;
            calculateSize();
        }

        // Self-Explaining i guess
        void calculateSize(){ size = max - min; }
    };

    //---------------------------------------------------------------------------
    //  MeshSuper class
    //---------------------------------------------------------------------------

    class MeshSuper
    {
    public:
        MeshSuper() {}
        virtual ~MeshSuper() {}

        // Bind this mesh (index & vertex-buffer) to the given cmd
        virtual void bind(VkCommandBuffer cmd) = 0;

        // Record command for drawing this mesh into the given cmd
        virtual void draw(VkCommandBuffer cmd) = 0;

        // Return the dimension of this mesh. Used for viewfrustum-culling.
        const Dimension& getDimension() const { return dimension; }

    protected:
        Dimension dimension; // Dimension of this Mesh

    };

    //---------------------------------------------------------------------------
    //  Mesh class
    //---------------------------------------------------------------------------

    // Represents a single mesh or a parent-mesh with submeshes
    class Mesh : public MeshSuper, public FileResourceObject
    {
        friend class AssimpLoader; // Allow the assimp-loader to access the private fields and fill it with data

    public:
        // Construct a new mesh object
        Mesh(const std::string& filePath);

        // You can pass a std::vector<> with different vertex formats directly instead of using a VertexBuffer object
        Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        ~Mesh();

        // Bind this mesh (index & vertex-buffer) to the given cmd
        void bind(VkCommandBuffer cmd) override;

        // Record command for drawing this mesh into the given cmd
        void draw(VkCommandBuffer cmd) override;

        // Getter's
        uint32_t                        getIndexBufferCount() const { return static_cast<uint32_t>(indices.size()); }
        const VulkanMeshResource*       getMeshResource() const { return meshResource; }

        const std::vector<SubMesh*>&    getSubMeshes() const { return subMeshes; }
        SubMesh*                        getSubMesh(uint32_t index){ return subMeshes[index]; }
        uint32_t                        numSubMeshes() const { return static_cast<uint32_t>(subMeshes.size()); }
        MaterialPtr                     getMaterial(uint32_t index);
        bool                            hasMaterials() const { return !materials.empty(); }
        uint32_t                        numMaterials() const { return static_cast<uint32_t>(materials.size()); }

    private:
        std::vector<Vertex>             vertices;           // Vertices describing this mesh
        std::vector<uint32_t>           indices;            // Indices describing this mesh

        VulkanMeshResource*             meshResource;       // Vulkan Mesh Resource (data on GPU)

        std::vector<SubMesh*>           subMeshes;          // SubMeshes which can have different materials
        std::vector<TexturePtr>         textures;           // Textures loaded from a mesh-file with materials
        std::map<uint32_t, MaterialPtr> materials;          // Key: Material-Index, Value: Pointer to a Material

        void uploadDataToGPU();
    };

    //---------------------------------------------------------------------------
    //  SubMesh class
    //---------------------------------------------------------------------------

    class SubMesh : public MeshSuper
    {
        friend class AssimpLoader; // Allow the assimp-loader to access the private fields and fill it with data

    public:
        SubMesh(Mesh* _parent) : parent(_parent){}
        ~SubMesh() {}

        uint32_t    startVertIndex;
        uint32_t    startIndex;
        uint32_t    numIndices;

        // Return the material used by this submesh
        MaterialPtr getMaterial();

        // Bind the parent-mesh (index & vertex-buffer) to the given cmd
        void bind(VkCommandBuffer cmd) override;

        // Record command for drawing this sub-mesh into the given cmd
        void draw(VkCommandBuffer cmd) override;

    private:
        Mesh*       parent;
        uint32_t    materialIndex;
    };

}


#endif // !MESH_H_
