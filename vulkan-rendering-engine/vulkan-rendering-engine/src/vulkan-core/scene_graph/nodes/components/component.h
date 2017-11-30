#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "vulkan-core/scene_graph/nodes/node.h"
#include <string>

namespace Pyro
{

    class Component
    {
        friend class Node; // Allow the Node-Class to access setParentNode() and remove()

    public:
        Component() : m_isActive(true) {};
        Component(bool isActive) : m_isActive(isActive) {};
        virtual ~Component() {}

        //Update the component
        virtual void update(float delta) = 0;
        virtual void lateUpdate(float delta) {}

        //Return the parent-node from this component
        Node* getParent() { return parentNode; }

        // Return whether this component should be updated
        bool isActive(){ return m_isActive; }

        // Enable / Disable this component from updating
        virtual void setIsActive(bool b){ m_isActive = b; }

        // Enable / Disable this component from updating
        virtual void toggleActive(){ m_isActive = !m_isActive; }

    protected:
        //The node this component is attached to
        Node* parentNode = nullptr;

        //Remove this component from a node. Called when removed from a Node.
        virtual void remove() { parentNode = nullptr; };

        //Called when this component is added to a node
        virtual void addedToNode(Node* node) {};

    private:
        //Set the parent node of this component. Called when added to a Node.
        void setParentNode(Node* node);

        //True: Component is active and will be updated from the parent-node
        bool m_isActive = true;

    };

}



#endif // !COMPONENT_H_
