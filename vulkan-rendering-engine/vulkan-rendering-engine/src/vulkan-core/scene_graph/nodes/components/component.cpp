#include "component.h"
#include "vulkan-core/vkTools/vk_tools.h"

namespace Pyro
{


    //Set the parent node of this component. Called when added to a Node.
    void Component::setParentNode(Node* node)
    {
        if (parentNode != nullptr)
            Logger::Log("Error in Component::setParentNode(): A Instanced Component can only attached to one object at a time!", LOGTYPE_ERROR);
        parentNode = node;

        this->addedToNode(node);
    }

}
