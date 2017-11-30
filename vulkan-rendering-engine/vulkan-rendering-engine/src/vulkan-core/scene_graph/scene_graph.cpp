#include "scene_graph.h"

#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"
#include "vulkan-core/memory_management/vulkan_memory_manager.h"
#include "vulkan-core/data/material/material.h"
#include "vulkan-core/vkTools/vk_tools.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Static Member
    //---------------------------------------------------------------------------

    Node SceneGraph::root(NODE_ROOT_NAME);                                      // The root object 

    std::vector<Renderable*>        SceneGraph::renderables;            // All renderable objects in the scene
    std::vector<Renderable*>        SceneGraph::staticNodes;            // All static objects in the scene
    std::vector<Renderable*>        SceneGraph::dynamicNodes;           // All dynamic objects in the scene
    std::vector<Node*>              SceneGraph::visibleObjects;         // All Visible Objects

    std::vector<Light*>             SceneGraph::lights;                 // All Lights in the scene
    std::vector<Light*>             SceneGraph::visibleLights;          // All currently visible lights in the scene
    std::vector<Light*>             SceneGraph::dirLights;              // All Directional-Lights in the scene
    std::vector<Light*>             SceneGraph::pointLights;            // All point-lights in the scene
    std::vector<Light*>             SceneGraph::spotLights;             // All spot-lights in the scene
    std::vector<PointLight*>        SceneGraph::visiblePointLights;     // All visible point-lights in the scene
    std::vector<SpotLight*>         SceneGraph::visibleSpotLights;      // All visible spot-lights in the scene

    //---------------------------------------------------------------------------
    //  Static Public Methods
    //---------------------------------------------------------------------------

    // Update the root object, which update all child objects and components
    void SceneGraph::update(float delta)
    {
        root.update(delta);
    }

    // Search for a node and returns it. Nullptr if none was found.
    Node* SceneGraph::findNode(uint32_t id)
    {
        return root.findNode(id);
    }

    // Search for a node and returns it. Nullptr if none was found.
    Node* SceneGraph::findNode(const std::string& name)
    {
        return root.findNode(name);
    }

    //---------------------------------------------------------------------------
    //  Static Private Methods - Node/Renderable Functions
    //---------------------------------------------------------------------------

    // Change the type of a node to static or dynamic
    void SceneGraph::changeNodeType(Renderable* renderable)
    {
        if (renderable->isStatic())
        {
            // Node was a dynamic before
            removeObjectFromList(dynamicNodes, renderable);
            staticNodes.push_back(renderable);
        }
        else {
            // Node was a static before
            removeObjectFromList(staticNodes, renderable);
            dynamicNodes.push_back(renderable);
        }
    }

    // Add a node to the scene-graph. - called from the Node-Class
    void SceneGraph::addNode(Node* node)
    {
        root.addChild(node);
    }

    void SceneGraph::removeNode(Node* node)
    {
        if (node->getName() != NODE_ROOT_NAME)
        {
            // Remove it from the Scene-Graph
            root.removeChild(node, false);

            // TODO: SEPARATE CLASS FOR CREATING AND DESTROYING OBJECTS AT RUNTIME
        }

    }

    void SceneGraph::addRenderable(Renderable* renderable)
    {
        // Add it to the Scene-Graph
        renderables.push_back(renderable);
        if (renderable->isStatic())
            staticNodes.push_back(renderable);
        else
            dynamicNodes.push_back(renderable);
    }

    // TODO: REMOVE ALL CHILDS ETC
    void SceneGraph::removeRenderable(Renderable* renderable)
    {
        // Remove it from the Scene-Graph
        removeObjectFromList(renderables, renderable);
        if (renderable->isStatic())
            removeObjectFromList(staticNodes, renderable);
        else
            removeObjectFromList(dynamicNodes, renderable);
    }

    // Cull all objects & lights in the scene-graph hierarchy
    void SceneGraph::cullLights(Frustum* frustum)
    {
        // Put all visible renderables now in a separate list
        visibleObjects.clear();
        for (auto& renderable : renderables)
            if (renderable->isVisible())
                visibleObjects.push_back(renderable);

        // Put all visible lights now in a separate list
        visibleLights.clear();
        visiblePointLights.clear();
        visibleSpotLights.clear();
        for (auto& light : lights)
        {
            if(!light->isActive())
                continue;

            light->cull(frustum);
            if (light->isVisible())
            {
                visibleLights.push_back(light);
                switch (light->getLightType())
                {
                case Light::Type::PointLight:
                    visiblePointLights.push_back(dynamic_cast<PointLight*>(light));
                    break;
                case Light::Type::SpotLight:
                    visibleSpotLights.push_back(dynamic_cast<SpotLight*>(light));
                    break;
                }
            }
        }
    }

    //---------------------------------------------------------------------------
    //  Static Private Methods - Lighting Functions
    //---------------------------------------------------------------------------

    void SceneGraph::addLight(Light* light)
    {
        lights.push_back(light);

        switch (light->getLightType())
        {
        case Light::Type::DirectionalLight:
            dirLights.push_back(light);
            break;
        case Light::Type::PointLight:
            pointLights.push_back(light);
            break;
        case Light::Type::SpotLight:
            spotLights.push_back(light);
            break;
        }
    }

    void SceneGraph::removeLight(Light* light)
    {
        removeObjectFromList(lights, light);
        switch (light->getLightType())
        {
        case Light::Type::DirectionalLight:
            removeObjectFromList(dirLights, light);
            break;
        case Light::Type::PointLight:
            removeObjectFromList(pointLights, light);
            break;
        case Light::Type::SpotLight:
            removeObjectFromList(spotLights, light);
            break;
        }
    }

}


