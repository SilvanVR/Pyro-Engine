#ifndef SCENE_H_
#define SCENE_H_

#include "vulkan-core/data/lighting/directional_light.h"
#include "vulkan-core/data/lighting/point_light.h"
#include "vulkan-core/data/lighting/spot_light.h"
#include "nodes/node.h"

#include "Input/input.h"
#include "time/time.h"

#include "vulkan-core/resource_manager/resource_manager.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class Cubemap;

    //---------------------------------------------------------------------------
    //  Scene Class
    //---------------------------------------------------------------------------

    class Scene
    {
        friend class Light;             // Access to addLight() & removeLight()
        friend class Node;              // Access to addNode() & removeNode()
        friend class Renderable;        // Access to addRenderable() & removeRenderable()
        friend class Object;            // Access to addObject() & removeObject()
        friend class SceneManager;      // Access to updateScene()

        // Updates all scene-objects and calls the standard update method for this scene
        void updateScene(float delta);

        // Called right before the scene gets loaded / unloaded
        void onCurrentSceneLoad(Scene* newCurrentScene) { root->onCurrentSceneLoad(newCurrentScene); }
        void onCurrentSceneUnload() { root->onCurrentSceneUnload(); }

    public:
        Scene(const std::string& name);
        virtual ~Scene();

        virtual void init(RenderingEngine* renderer) = 0;
        virtual void update(float delta) {}
        virtual void lateUpdate(float delta) {}

        // Called when the scene is already in memory and should be made active
        // It defaults search for a camera in the scene and activates it
        // Can be overriden to e.g. set the renderer-state again if needed
        virtual void reload(RenderingEngine* renderer);

        const std::string&              getName() const { return sceneName; }
        Node*                           getRoot() { return root; }
        const std::vector<Renderable*>& getAllRenderables() const { return renderables; }
        std::vector<Renderable*>        getRenderables(LayerMask layerMask = LayerMask({ LAYER_DEFAULT })) const;
        const std::vector<Light*>&      getLights() const { return lights; }
        const std::vector<Light*>&      getDirectionalLights() const { return dirLights; }
        const std::vector<Light*>&      getPointLights() const { return pointLights; }
        const std::vector<Light*>&      getSpotLights() const { return spotLights; }
        std::vector<Renderable*>        getRenderablesWithinRadius(const Point3f& pos, float radius, LayerMask layerMask = LayerMask({LAYER_DEFAULT}));
        std::vector<Node*>              getGlobalNodes();

        Node*       findNode(const std::string& name);
        Renderable* findRenderable(const std::string& name);

    protected:
        // Attach the given function to the input-system. All callbacks gets cleaned up when the scene gets destroyed.
        void attachInputFunc(int keyCode, const std::function<void()>& func, Input::ECallbackEvent event = Input::ECallbackEvent::KEY_PRESSED);

        // Attach the given function to the timer-system. Calls the function every x-ms. Get's cleaned up when scene gets destroyed.
        void setInterval(const std::function<void()>& func, uint64_t ms);

        // Attach the given function to the timer-system. Calls the function once after x-ms. Pending timeout will be cleaned up on scene destruction.
        void setTimeout(const std::function<void()>& func, uint64_t ms);

    private:
        // forbid copy & copy assignment
        Scene(const Scene& scene) = delete;
        Scene& operator=(const Scene& scene) = delete;

        std::string                 sceneName;          // The name of this scene

        Node*                       root;               // The root-object of this scene-graph
        std::vector<Object*>        objects;            // All Object bound to this scene (materials, textures, nodes etc.)
        std::vector<Renderable*>    renderables;        // All renderable objects in the scene

        // Lights
        std::vector<Light*>         lights;             // All lights in the scene
        std::vector<Light*>         dirLights;          // All Directional-Lights in the scene
        std::vector<Light*>         pointLights;        // All point-lights in the scene
        std::vector<Light*>         spotLights;         // All spot-lights in the scene

        // Callback IDs
        std::vector<CallbackID>     inputCallbackIDs;
        std::vector<CallbackID>     timerCallbackIDs;

        // Resource-Mappers
        friend class TextureManager; // Access to mapTexture(), getTextureID(), removeTextureID()
        ResourceMapper textureMapper;
        bool mapTexture(ResourceID id, MappingValuePtr val){ return textureMapper.add(id, val); }
        ResourceID getTextureID(MappingValuePtr val){ return textureMapper.get(val); }
        void removeTextureID(ResourceID id){ textureMapper.remove(id); }

        // Node functions   
        void addRenderable(Renderable* renderable);      // Add a renderable to the scene-graph
        void removeRenderable(Renderable* renderable);   // Remove a renderable from the scene-graph

        // Light functions
        void addLight(Light* light);                     // Add a light to the scene-graph
        void removeLight(Light* light);                  // remove a light from the scene-graph

        // Object functions
        void addObject(Object* obj);
        void removeObject(Object* obj);

        void addNodeToRoot(Node* node);
        void transferGlobalObjects(Scene* newScene);
    };

    //---------------------------------------------------------------------------
    //  Empty-Scene Class
    //---------------------------------------------------------------------------

    class EmptyScene : public Scene
    {
    public:
        EmptyScene() : Scene("EmptyScene") {}
        ~EmptyScene() {}

        void init(RenderingEngine* renderer) override;
    };

}


#endif // !SCENE_H_
