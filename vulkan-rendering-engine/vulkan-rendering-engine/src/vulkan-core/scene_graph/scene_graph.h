#ifndef SCENE_GRAPH_H_
#define SCENE_GRAPH_H_

#include "vulkan-core/data/lighting/directional_light.h"
#include "vulkan-core/data/lighting/point_light.h"
#include "vulkan-core/data/lighting/spot_light.h"
#include "nodes/node.h"

namespace Pyro
{

    class Renderable;

    //---------------------------------------------------------------------------
    //  SceneGraph class
    //---------------------------------------------------------------------------

    class SceneGraph
    {
        friend class Light;             //Allow a light to add themselve to this scene graph
        friend class Node;              //Allow node to add themselve to this scene graph (to the root-object as a child)
        friend class Renderable;        //Allow a Renderable to add themselve to this scene graph

    public:
        SceneGraph() {}
        ~SceneGraph() {}

        static void update(float delta);    // Update all Node-objects in the scene

        static Node*                                  getRoot() { return &root; }
        static const std::vector<Renderable*>&        getRenderables() { return renderables; }
        static const std::vector<Renderable*>&        getDynamicNodes() { return dynamicNodes; }
        static const std::vector<Renderable*>&        getStaticNodes() { return staticNodes; }
        static const std::vector<Node*>               getVisibleObjects() { return visibleObjects; }
        static const std::vector<Light*>&             getLights() { return lights; }
        static const std::vector<Light*>&             getVisibleLights() { return visibleLights; }
        static const std::vector<Light*>&             getDirectionalLights(){ return dirLights; }
        static const std::vector<Light*>&             getPointLights() { return pointLights; }
        static const std::vector<Light*>&             getSpotLights() { return spotLights; }
        static const std::vector<PointLight*>&        getVisiblePointLights() { return visiblePointLights; }
        static const std::vector<SpotLight*>&         getVisibleSpotLights() { return visibleSpotLights; }

        static Node*    findNode(uint32_t id);                  // Find a node in the scene-graph and return it.
        static Node*    findNode(const std::string& name);      // Find a node in the scene-graph and return it.

        static void cullLights(Frustum* frustum);               // Cull lights in the scene-graph hierarchy and fill culling-lists

    private:
        static Node                     root;                   // The root-object of this scene-graph

        static std::vector<Node*>       visibleObjects;         // All Objects who are within the view-frustum
        static std::vector<Renderable*> staticNodes;            // All static objects in the scene.
        static std::vector<Renderable*> dynamicNodes;           // All dynamic objects in the scene

        static std::vector<Renderable*> renderables;            // All renderable objects in the scene

        // Lights
        static std::vector<Light*>      lights;                 // All lights in the scene
        static std::vector<Light*>      visibleLights;          // All currently visible lights in the scene
        static std::vector<Light*>      dirLights;              // All Directional-Lights in the scene
        static std::vector<Light*>      pointLights;            // All point-lights in the scene
        static std::vector<Light*>      spotLights;             // All spot-lights in the scene
        static std::vector<PointLight*> visiblePointLights;     // All visible point-lights in the scene
        static std::vector<SpotLight*>  visibleSpotLights;      // All visible spot-lights in the scene

        // Node functions
        static void addNode(Node* node);                        // Add a node to the scene-graph
        static void removeNode(Node* node);                     // Remove a node from the scene-graph
        static void addRenderable(Renderable* renderable);      // Add a renderable to the scene-graph
        static void removeRenderable(Renderable* renderable);   // Remove a renderable from the scene-graph
        static void changeNodeType(Renderable* renderable);     // Change the type of a node to static or dynamic

        // Light functions
        static void addLight(Light* light);                     // Add a light to the scene-graph
        static void removeLight(Light* light);                  // remove a light from the scene-graph


    };

}




#endif // !SCENE_GRAPH_H_
