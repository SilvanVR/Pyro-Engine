#ifndef RENDERABLE_H_
#define RENDERABLE_H_

#include "vulkan-core/resource_manager/resource.hpp"
#include "vulkan-core/scene_graph/nodes/node.h"
#include "vulkan-core/data/material/material.h"
#include "vulkan-core/data/mesh/mesh.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Renderable class
    //---------------------------------------------------------------------------

    class Renderable : public Node
    {
        Renderable(Renderable* parent, MeshPtr mesh, uint32_t meshIndex, MaterialPtr material, const Transform& transform, EType type, bool addCollider);

    public:
        // Create a renderable basically from a mesh and a material
        Renderable(const std::string& name, MeshPtr mesh, MaterialPtr material, const Transform& transform = Transform(), EType type = EType::Dynamic, bool addCollider = true);
        Renderable(MeshPtr mesh, MaterialPtr material, const Transform& transform = Transform(), EType type = EType::Dynamic, bool addCollider = true);

        // Create a renderable from a mesh with submeshes and materials contained in the mesh-class (mesh HAS TO HAVE materials)
        Renderable(MeshPtr mesh, const Transform& transform = Transform(), EType type = EType::Dynamic);
        virtual ~Renderable();

        void render(VkCommandBuffer cmd, ShaderPtr shader) override;

        // Cull this object (mesh)
        bool cull(Frustum* frustum) override;

        MeshPtr         getMesh() { return m_mesh; }
        MaterialPtr     getMaterial() { return m_material; }
        void            setMesh(MeshPtr mesh, bool addCollider = true);

        // Change the material. Default material if material == nullptr.
        void setMaterial(MaterialPtr material);

    protected:
        MeshPtr     m_mesh;           // The mesh this renderable is using
        MaterialPtr m_material;       // The material this renderable is using

    private:
        //forbid copy and copy assignment
        Renderable(const Renderable& renderable) = delete;
        Renderable& operator=(const Renderable& renderable) = delete;

        uint32_t m_meshIndex;
        Renderable* m_parent;
        std::vector<Renderable*> subRenderables;

        void createSubRenderables();

        void removeSubRenderable(Renderable* renderable);
    };

}


#endif // !RENDERABLE_H_
