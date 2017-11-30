#ifndef NODE_H_
#define NODE_H_

#include "../object.h"
#include "build_options.h"
#include "vulkan-core/scene_graph/layers/layer_mask.h"
#include "vulkan-core/resource_manager/resource.hpp"
#include "transform.h"
#include <vector>
#include <string>

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define NODE_ROOT_NAME "ROOT"

    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class Frustum;
    class Shader;
    class Component;

    //---------------------------------------------------------------------------
    //  Node class
    //---------------------------------------------------------------------------

    class Node : public Object
    {
        friend class Scene; // Allow the scene to construct the root-object using the constructor below
        Node(const std::string& name, Scene* scene); // Constructor for the ROOT-Object
         
    public:
        enum EType
        {
            Static,     // Static object (e.g. light shadow-map gets rendered only once, world-matrix never gets recalculated)
            Dynamic     // Dynamic object
        };

        // Constructor & Destructor
        Node(const std::string& name, EType type = EType::Dynamic);
        Node(const std::string& name, const Point3f& pos, EType type = EType::Dynamic);
        Node(const std::string& name, const Transform& transform, EType type = EType::Dynamic);
        virtual ~Node();

        // Update the Node and all child-objects
        virtual void update(float delta);
        virtual void lateUpdate(float delta);

        // Called once when the scene has been fully built
        virtual void onCurrentSceneLoad(Scene* newCurrentScene);

        // Called once right before the scene gets deleted
        virtual void onCurrentSceneUnload();

        // Record commands which render this node (but not every node can be rendered)
        virtual void render(VkCommandBuffer cmd, Resource<Shader> shader)  {}

        // Cull objects in the scene-graph hierarchy. True means the object is visible
        virtual bool cull(Frustum* frustum) { return true; }

        // Toggle the type between dynamic and static.
        virtual void toggleType();

        // Getter's
        const Mat4f&            getWorldMatrix();
        const Point3f&          getWorldPosition();
        const Quatf&            getWorldRotation();
        const Vec3f&            getWorldScale();
        const Transform&        getWorldTransform() const { return worldTransform; }

        const Point3f&          getLocalPosition(){ return transform.position; }
        const Vec3f&            getLocalScale() { return transform.scale; }
        const Quatf&            getLocalRotation() { return transform.rotation; }

        bool                    isStatic() { return type == EType::Static; }
        bool                    isActive() { return m_isActive; }
        std::vector<Node*>&     getChildren() { return children; }
        Node*                   getParent() { return parent; }

        void                    setIsActive(bool newIsActive);
        void                    toggleActive();
        bool                    attachedToRoot();

        void getGlobalNodes(std::vector<Node*>& globals);

        // Returns the local-transform of the node. Set's the dirty-flag -> next time the World-Matrix is needed it will be calculated.
        // Never store the transform in a variable. Always retrieve it via this method.
        Transform&              getTransform();

        // Transform Hierarchy
        Node*                   findNode(const std::string& name);
        void                    setParent(Node* parent, bool keepWorldTransform = true);
        void                    removeParent();
        void                    addChild(Node* child);
        void                    removeChild(Node* child, bool keepWorldTransform = true);

        // Components
        void addComponent(Component* component);
        void removeComponent(Component* component);
        const std::vector<Component*>& getComponents() { return components; }

        // Return the in brackets specified component. Nullptr if component wasnt found.
        template <typename T> T* getComponent();

        // Search and remove the given component. True if successful, false otherwise.
        template <typename T> bool removeComponent();

        // Layer functions
        const LayerMask& getLayerMask() { return layerMask; }
        virtual void changeLayer(const std::vector<std::string>& names) { layerMask.changeLayer(names); }
        virtual void addLayer(const std::string& name) { layerMask.addLayer(name); }
        virtual void removeLayer(const std::string& name) { layerMask.removeLayer(name); }

    protected:
        Node*                   parent;             // Parent-Node. Only the "Root"-object has no parent-node.
        EType                   type;               // The type of this object
        bool                    m_isActive;         // True if this object and his childs/components should be updated/rendered
        LayerMask               layerMask;          // Every object belongs to zero or several layers.
        bool                    wmIsDirty;          // Tells if the world-matrix is up to date or not (dirty)

    private:
        std::vector<Node*>          children;
        std::vector<Component*>     components;

        // Calculates the world matrix
        void calculateWorldMatrix();

        // Cached world-matrix
        Mat4f       worldMatrix;

        // Transform for this object
        Transform   transform;

        // Transform in world-coordinates
        Transform   worldTransform;

        // Return the inherited rotation in world-space.
        Quatf       getInheritedRotation();

        void setWorldMatrixIsDirty();

        //---------------------------------------------------------------------------
        //  Interpolation stuff
        //---------------------------------------------------------------------------

        //// Last world matrix, current and (interpolated) render matrix
        //Mat4f       lastWorldMatrix, renderMatrix;

        //// Last Rotation
        //Quatf       lastRotation;

        //// Contains the interpolated values for scale, rotation and translation
        //Transform   renderTransform;

        //const Mat4f&            getInterpolatedWorldMatrix() { return renderMatrix; }
        //const Point3f&          getInterpolatedWorldPosition() { return renderTransform.position; }
        //const Quatf&            getInterpolatedWorldRotation() { return renderTransform.rotation; }
        //const Vec3f&            getInterpolatedWorldScale() { return renderTransform.scale; }

        //// Interpolation Methods
        //Quatf       getInterpolatedRotation(float interpolation);
        //Point3f     getInterpolatedPosition(float interpolation);
        //Vec3f       getInterpolatedScale(float interpolation);

        //Point3f     getLastWorldPosition();
        //Quatf       getLastWorldRotation();
        //Vec3f       getLastWorldScale();

    };

    //---------------------------------------------------------------------------
    //  Template Component Methods
    //---------------------------------------------------------------------------

    // Return the in angle brackets specified component. Nullptr if component wasnt found.
    template <typename T>
    T* Node::getComponent()
    {
        for (Component* component : components)
        {
            T* comp = dynamic_cast<T*>(component);

            if (comp != nullptr)
                return comp;
        }
        return nullptr;
    }

    // Search and remove the given component. True if successful, false otherwise.
    template <typename T>
    bool Node::removeComponent()
    {
        for (Component* component : components)
        {
            T* comp = dynamic_cast<T*>(component);

            if (comp != nullptr)
            {
                components.erase(std::remove(components.begin(), components.end(), comp), components.end());
                delete comp;
                return true;
            }
        }
        return false;
    }

}

#endif // !NODE_H_
