#include "renderable.h"

#include "vulkan-core/scene_graph/nodes/components/colliders/sphere_collider.h"
#include "vulkan-core/scene_graph/scene_manager.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------

    Renderable::Renderable(MeshPtr mesh, MaterialPtr material, const Transform& transform, EType type, bool addCollider)
        : Renderable("Renderable", mesh, material, transform, type, addCollider)
    {}

    Renderable::Renderable(const std::string& name, MeshPtr mesh, MaterialPtr material, const Transform& transform, EType type, bool addCollider)
        : Node(name, transform, type), m_mesh(mesh), m_material(material)
    {
        assert(mesh.isValid());
        layerMask.addLayer(LAYER_DEFAULT);

        setMesh(mesh, addCollider);
    }

    // Create a renderable with submeshes and materials contained in the mesh-class (if the mesh has no material the default one will applied)
    Renderable::Renderable(MeshPtr mesh, const Transform& transform, EType type)
        : Renderable(mesh, nullptr, transform, type)
    {
        bool hasMaterials = m_mesh->hasMaterials();
        if (!hasMaterials){
            Logger::Log("Renderable::Renderable(): Mesh #" + m_mesh->getFilePath() + " has ho materials. Using the default one instead. "
                        " If you want to use another material pass it in in the constructor", LOGTYPE_WARNING);
        }
    }

    // Constructor for SubRenderables
    Renderable::Renderable(Renderable* parent, MeshPtr mesh, uint32_t meshIndex, MaterialPtr _material, const Transform& transform, EType type, bool addCollider)
        : Node(parent->getName(), transform, type), m_parent(parent), m_mesh(mesh), m_material(_material), m_meshIndex(meshIndex)
    {
        layerMask.addLayer(LAYER_DEFAULT);

        float radius = m_mesh->getSubMesh(meshIndex)->getDimension().maxRadius;
        if(addCollider) 
            addComponent(new SphereCollider(radius));
        m_material->addRenderable(this);
        SceneManager::getCurrentScene()->addRenderable(this);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Renderable::~Renderable()
    {
        SceneManager::getCurrentScene()->removeRenderable(this);

        if(m_material.isValid())
            m_material->removeRenderable(this);

        if(m_parent != nullptr)
            m_parent->removeSubRenderable(this);

        while(!subRenderables.empty())
            delete subRenderables.front();
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void Renderable::setMesh(MeshPtr mesh, bool addCollider)
    { 
        while (!subRenderables.empty())
            delete subRenderables.front();

        m_mesh = mesh;
        auto& subMeshes = m_mesh->getSubMeshes();
        if (subMeshes.size() > 1)
        {
            createSubRenderables();
        }
        else
        {
            if (!m_material.isValid())
                m_material = MATERIAL_GET(MATERIAL_DEFAULT);

            if (addCollider)
            {
                removeComponent<SphereCollider>();
                addComponent(new SphereCollider(mesh));
            }
            m_material->addRenderable(this);
            SceneManager::getCurrentScene()->addRenderable(this);
        }
    }

    void Renderable::setMaterial(MaterialPtr material)
    {
        if (m_material.isValid()) // a parent object hasn't a material
        {
            m_material->removeRenderable(this);
            m_material = material.isValid() ? material : MATERIAL_GET(MATERIAL_DEFAULT);
            m_material->addRenderable(this);
        }
        for (auto& subRenderable : subRenderables)
            subRenderable->setMaterial(material);
    }

    void Renderable::render(VkCommandBuffer cmd, ShaderPtr shader)
    {
        if (m_parent != nullptr)
        {
            // Bind Index- & Vertex-Buffer
            m_mesh->getSubMesh(m_meshIndex)->bind(cmd);

            // Update per object data through push-constant
            shader->pushConstant(cmd, 0, sizeof(Mat4f), &getWorldMatrix());

            // Draw indexed mesh (with perhaps several submeshes)
            m_mesh->getSubMesh(m_meshIndex)->draw(cmd);
        }
        else
        {
            // Bind Index- & Vertex-Buffer
            m_mesh->bind(cmd);

            // Update per object data through push-constant
            shader->pushConstant(cmd, 0, sizeof(Mat4f), &getWorldMatrix());

            // Draw indexed mesh (with perhaps several submeshes)
            m_mesh->draw(cmd);
        }

    }

    bool Renderable::cull(Frustum* frustum)
    {
        // Check if the sphere around this object is within the view-frustum
        auto col = getComponent<SphereCollider>();
        if(!col) return true;
        return frustum->checkSphere(col);
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    void Renderable::createSubRenderables()
    {
        bool meshHasMaterials = m_mesh->hasMaterials();
        auto& subMeshes = m_mesh->getSubMeshes();
        for (int i = 0; i < subMeshes.size(); i++)
        {
            const Point3f& localPos = subMeshes[i]->getDimension().localPosition;
            MaterialPtr subRenderableMaterial;

            bool providedMaterial = m_material.isValid();
            if (providedMaterial)
            {
                subRenderableMaterial = m_material;
            }
            else
            {
                if (meshHasMaterials) {
                    subRenderableMaterial = subMeshes[i]->getMaterial();
                } else {
                    subRenderableMaterial = MATERIAL_GET(MATERIAL_DEFAULT);
                }
            }

            Renderable* subRenderable = new Renderable(this, m_mesh, i, subRenderableMaterial, Transform(localPos), type, true);
            addChild(subRenderable);
            subRenderables.push_back(subRenderable);
        }
    }

    void Renderable::removeSubRenderable(Renderable* renderable)
    {
        removeObjectFromList(subRenderables, renderable);
        removeChild(renderable);
    }

}