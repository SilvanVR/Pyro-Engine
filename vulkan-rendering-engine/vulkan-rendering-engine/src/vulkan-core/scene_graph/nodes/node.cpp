#include "node.h"

#include "vulkan-core/scene_graph/scene_manager.h"
#include "components/component.h"
#include <assert.h>

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------

    Node::Node(const std::string& name, Scene* scene)
        : Object(Object::LOCAL, name, scene), parent(nullptr), m_isActive(true), type(EType::Dynamic), wmIsDirty(true)
    {
        calculateWorldMatrix();
    }

    Node::Node(const std::string& name, EType type)
        : Node(name, Point3f(), type)
    {}

    Node::Node(const std::string& name, const Point3f& pos, EType type)
        : Node(name, Transform(pos), type)
    {}

    Node::Node(const std::string& _name, const Transform& _transform, EType _type)
        : Object(Object::LOCAL, _name),
          transform(_transform), parent(nullptr), m_isActive(true), type(_type), wmIsDirty(true)
    {
        SceneManager::getCurrentScene()->addNodeToRoot(this);
        calculateWorldMatrix();
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Node::~Node()
    {
        for (Component* component : components)
            delete component;

        if (parent != nullptr)
            parent->removeChild(this);

        // Delete all children of that node
        while (!children.empty())
            delete children.front();
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Update the whole scene graph hierarchy. Called on the root
    void Node::update(float delta)
    {
        for (uint32_t i = 0; i < components.size(); i++)
            if (components[i]->isActive())
                components[i]->update(delta);

        for (uint32_t i = 0; i < children.size(); i++)
            if (children[i]->isActive())
                children[i]->update(delta);
    }

    // Update the whole scene graph hierarchy after the normal update
    void Node::lateUpdate(float delta)
    {
        for (uint32_t i = 0; i < components.size(); i++)
            if (components[i]->isActive())
                components[i]->lateUpdate(delta);

        for (uint32_t i = 0; i < children.size(); i++)
            if (children[i]->isActive())
                children[i]->lateUpdate(delta);
    }
    
    void Node::onCurrentSceneLoad(Scene* newCurrentScene)
    {
        for (uint32_t i = 0; i < children.size(); i++)
            if (children[i]->isActive())
                children[i]->onCurrentSceneLoad(newCurrentScene);
    }

    void Node::onCurrentSceneUnload()
    {
        for (uint32_t i = 0; i < children.size(); i++)
            if (children[i]->isActive()) 
                children[i]->onCurrentSceneUnload();
    }

    //---------------------------------------------------------------------------
    //  Public Methods - Transform Hierarchy
    //---------------------------------------------------------------------------

    Transform& Node::getTransform() 
    {
        setWorldMatrixIsDirty();
        return transform; 
    }

    const Mat4f& Node::getWorldMatrix()
    {
        calculateWorldMatrix();
        return worldMatrix; 
    }

    const Point3f& Node::getWorldPosition()
    {
        calculateWorldMatrix();
        return worldTransform.position; 
    }

    const Quatf& Node::getWorldRotation()
    { 
        calculateWorldMatrix();
        return worldTransform.rotation; 
    }

    const Vec3f& Node::getWorldScale()
    { 
        calculateWorldMatrix();
        return worldTransform.scale; 
    }

    // Search a node in the hierarchy and return it. Nullptr if none was found.
    Node* Node::findNode(const std::string& name)
    {
        if (getName() == name)
            return this;

        Node* foundNode = nullptr;
        for (Node* child : children)
        {
            foundNode = child->findNode(name);
            if(foundNode != nullptr)
                return foundNode;
        }

        return foundNode;
    }

    // Set's a new parent. Keep the world transform if keepWorldTransform = true.
    void Node::setParent(Node* parent, bool keepWorldTransform)
    {
        if (this->parent != nullptr) //remove child from last parent
            this->parent->removeChild(this, keepWorldTransform);

        this->parent = parent;
        if(parent) this->parent->children.push_back(this);

        setWorldMatrixIsDirty();
    }

    // Remove the parent (set the parent to the root) and remove this one from the child-list of the parent.
    void Node::removeParent()
    {
        if (this->parent == SceneManager::getCurrentScene()->getRoot() || parent == this)
            return;

        parent->removeChild(this, false);
        this->parent = SceneManager::getCurrentScene()->getRoot();

        setWorldMatrixIsDirty();
    }

    // Add the given child. The child inherits the transform from the parent.
    void Node::addChild(Node* child)
    {
        if (child->parent != nullptr) //remove child from last parent
            child->parent->removeChild(child, false);

        child->parent = this;
        children.push_back(child);

        child->setWorldMatrixIsDirty();
    }

    // Remove the given child and set's his parent to the ROOT. Keep's the world transform if keepWorldTransform = true.
    void Node::removeChild(Node* child, bool keepWorldTransform)
    {
        if (child == this)
            return;

        for (unsigned int i = 0; i < children.size(); i++)
        {
            if (children[i] == child)
            {
                if (keepWorldTransform)
                    children[i]->transform = children[i]->getWorldTransform();
                children[i]->parent = SceneManager::getCurrentScene()->getRoot();
                children.erase(children.begin() + i);
                break;
            }
        }
        child->setWorldMatrixIsDirty();
    }

    //---------------------------------------------------------------------------
    //  Public Methods - Components
    //---------------------------------------------------------------------------

    void Node::addComponent(Component* component)
    {
        this->components.push_back(component);
        component->setParentNode(this);
    }

    void Node::removeComponent(Component* component)
    {
        for (unsigned int i = 0; i < components.size(); i++)
        {
            if (components[i] == component)
            {
                components[i]->remove();
                components.erase(components.begin() + i);
                break;
            }
        }
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void Node::getGlobalNodes(std::vector<Node*>& globals)
    {
        if(isGlobal()) globals.push_back(this);
        for(auto& child : children)
            child->getGlobalNodes(globals);
    }

    void Node::toggleType()
    {
        if (type == EType::Dynamic)
        {
            type = EType::Static;
            calculateWorldMatrix();
        }
        else
            type = EType::Dynamic;
    }

    void Node::setIsActive(bool newIsActive)
    { 
        m_isActive = newIsActive;
        for(auto& child : children)
            child->setIsActive(m_isActive);

        for (auto& c : components)
            c->setIsActive(m_isActive);
    }

    void Node::toggleActive() 
    { 
        m_isActive = !m_isActive;
        for (auto& child : children)
            child->setIsActive(m_isActive);

        for (auto& c : components)
            c->setIsActive(m_isActive);
    }

    bool Node::attachedToRoot() 
    { 
        return parent == SceneManager::getCurrentScene()->getRoot(); 
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Calculates the world matrix based on the position in the transform - hierarchy, but only if the matrix is dirty.
    void Node::calculateWorldMatrix()
    {
        // World-Matrix up-to-date? Object is static?
        if (isStatic() || !wmIsDirty) return;

        if (parent == nullptr) // root node
            worldMatrix = transform.getTransformationMatrix();
        else
            worldMatrix = parent->getWorldMatrix() * transform.getTransformationMatrix();

        // Retrieve the position, rotation & scale from the world-matrix
        worldTransform.position = static_cast<Point3f>(worldMatrix.getTranslation());
        worldTransform.scale    = worldMatrix.getScale();
        worldTransform.rotation = getInheritedRotation(); 

        // World-Matrix is up-to-date now
        wmIsDirty = false;
    }

    // Return the inherited rotation in world-space.
    // Reason for this method is that retrieving the rotation from 
    // the world-matrix makes problems if the scale is negative.
    Quatf Node::getInheritedRotation()
    {
        if(parent == nullptr)
            return transform.rotation;

        return parent->getInheritedRotation() * transform.rotation;
    }


    void Node::setWorldMatrixIsDirty()
    {
        wmIsDirty = true;
        for(auto& child : children)
            child->setWorldMatrixIsDirty();
    }


    //---------------------------------------------------------------------------
    //  Interpolation Stuff
    //---------------------------------------------------------------------------

    //// Return the last world position of the object in world coordinates
    //Point3f Node::getLastWorldPosition()
    //{
    //    return static_cast<Point3f>(lastWorldMatrix.getTranslation());
    //}

    //// Return the last rotation of the object in world coordinates.
    //Quatf Node::getLastWorldRotation()
    //{
    //    if (parent == nullptr) //reached root node
    //        return lastRotation;

    //    return parent->getLastWorldRotation() * lastRotation;
    //}

    //// Return the last scale of the object in world coordinates
    //Vec3f Node::getLastWorldScale()
    //{
    //    return lastWorldMatrix.getScale();
    //}

    //Quatf Node::getInterpolatedRotation(float interpolation)
    //{
    //    const Quatf& lastQuat = getLastWorldRotation();
    //    const Quatf& currentQuat = getWorldRotation();

    //    return lastQuat.slerp(currentQuat, interpolation);
    //}

    //Point3f Node::getInterpolatedPosition(float interpolation)
    //{
    //    const Point3f& lastTranslation = getLastWorldPosition();
    //    const Point3f& currentTranslation = getWorldPosition();

    //    return Mathf::lerp(lastTranslation, currentTranslation, interpolation);
    //}

    //Vec3f Node::getInterpolatedScale(float interpolation)
    //{
    //    const Vec3f& lastScale = getLastWorldScale();
    //    const Vec3f& currentScale = getWorldScale();

    //    return Mathf::lerp(lastScale, currentScale, interpolation);
    //}


    // Calculates the interpolated world-matrix used for rendering
    //void Node::calcIntData(float interpolation)
    //{
    //    // Calculate render-matrix if the object is a dynamic
    //    if (type == EType::Dynamic)
    //    {
    //        renderTransform.scale = getInterpolatedScale(interpolation);
    //        renderTransform.position = getInterpolatedPosition(interpolation);
    //        renderTransform.rotation = getInterpolatedRotation(interpolation);
    //        renderMatrix = renderTransform.getTransformationMatrix();
    //    }

    //    for (Node* child : children)
    //        if (child->isActive())
    //            child->calcIntData(interpolation);
    //}











}