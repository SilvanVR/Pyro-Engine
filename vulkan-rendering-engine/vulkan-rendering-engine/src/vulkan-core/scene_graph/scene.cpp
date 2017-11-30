#include "scene.h"

#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"
#include "vulkan-core/data/material/texture/cubemap.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "vulkan-core/rendering_engine.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Scene::Scene(const std::string& _name)
        : sceneName(_name)
    {
        // Will be cleaned up in destructor automatically
        root = new Node(sceneName + "#" + NODE_ROOT_NAME, this);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Scene::~Scene()
    {
        // Delete all remaining nodes on the heap in the scene
        while (!objects.empty())
        {
            Object* obj = objects.front();
            delete obj; // It removes itself from the "object-vector"
        }

        // Detach all input-functions from the input-system
        for(auto callbackID : inputCallbackIDs)
            Input::detachFunc(callbackID);

        // Detach all Timer-Functions from the timer-system
        for (auto callbackID : timerCallbackIDs)
            Time::clearCallback(callbackID);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    Node* Scene::findNode(const std::string& name)
    {
        return root->findNode(name);
    }

    Renderable* Scene::findRenderable(const std::string& name)
    {
        return dynamic_cast<Renderable*>(findNode(name));
    }

    std::vector<Renderable*> Scene::getRenderablesWithinRadius(const Point3f& pos, float radius, LayerMask layerMask)
    {
        std::vector<Renderable*> renderablesWithinRadius;
        for (auto& r : renderables)
        {
            if(!(layerMask & r->getLayerMask())) continue;

            float distance = r->getWorldPosition().distance(pos);
            if (distance < radius)
                renderablesWithinRadius.push_back(r);
        }

        return renderablesWithinRadius;
    }

    std::vector<Renderable*> Scene::getRenderables(LayerMask layerMask) const
    {
        std::vector<Renderable*> result;
        for(auto& r : renderables)
            if(r->getLayerMask() & layerMask)
                result.push_back(r);
        return result;
    }

    std::vector<Node*> Scene::getGlobalNodes()
    {
        std::vector<Node*> globals;
        root->getGlobalNodes(globals);
        return globals;
    }

    void Scene::reload(RenderingEngine* renderer) 
    {
        // Try to find a camera in the scene 
        Camera* cam = dynamic_cast<Camera*>(findNode("Camera"));

        if(cam)
            renderer->setCamera(cam);
        else
            Logger::Log("Scene::reload(): Could not find a camera", LOGTYPE_ERROR);
    };

    //---------------------------------------------------------------------------
    //  Protected Methods
    //---------------------------------------------------------------------------

    void Scene::attachInputFunc(int keyCode, const std::function<void()>& func, Input::ECallbackEvent event)
    {
        CallbackID id = Input::attachFunc(keyCode, func, event);
        inputCallbackIDs.push_back(id);
    }

    void Scene::setInterval(const std::function<void()>& func, uint64_t ms)
    {
        CallbackID id = Time::setInterval(func, ms);
        timerCallbackIDs.push_back(id);
    }

    void Scene::setTimeout(const std::function<void()>& func, uint64_t ms)
    {
        CallbackID id = Time::setTimeout(func, ms);
        timerCallbackIDs.push_back(id);
    }

    //---------------------------------------------------------------------------
    //  Private Friend Methods
    //---------------------------------------------------------------------------

    void Scene::updateScene(float delta)
    {
        this->update(delta);
        root->update(delta);
        root->lateUpdate(delta);
    }

    void Scene::transferGlobalObjects(Scene* newScene)
    {
        // Add all global objects from the previous scene to the new scene
        for (auto& node : getGlobalNodes())
        {
            // Attach the node to the new root
            if (node->attachedToRoot())
                newScene->addNodeToRoot(node);

            // Renderables + Lights must be added separately
            Renderable* r = dynamic_cast<Renderable*>(node);
            if (r) newScene->addRenderable(r);
            Light* l = dynamic_cast<Light*>(node);
            if (l) newScene->addLight(l);
        }
    }
    
    //---------------------------------------------------------------------------
    //  Static Private Methods - Node/Renderable Functions
    //---------------------------------------------------------------------------

    void Scene::addNodeToRoot(Node* node)
    {
        root->addChild(node);
    }

    void Scene::addObject(Object* obj)
    { 
        objects.push_back(obj); 
    }

    void Scene::removeObject(Object* obj)
    {
        removeObjectFromList(objects, obj);
    }

    void Scene::addRenderable(Renderable* renderable)
    {
        renderables.push_back(renderable);
    }

    // TODO: REMOVE ALL CHILDS ETC
    void Scene::removeRenderable(Renderable* renderable)
    {
        // Remove it from the Scene-Graph
        removeObjectFromList(renderables, renderable);
    }

    //---------------------------------------------------------------------------
    //  Private Methods - Lighting Functions
    //---------------------------------------------------------------------------

    void Scene::addLight(Light* light)
    {
        lights.push_back(light);

        switch (light->getLightType())
        {
        case Light::DirectionalLight:
            dirLights.push_back(light);
            break;
        case Light::PointLight:
            pointLights.push_back(light);
            break;
        case Light::SpotLight:
            spotLights.push_back(light);
            break;
        }
    }

    void Scene::removeLight(Light* light)
    {
        removeObjectFromList(lights, light);
        switch (light->getLightType())
        {
        case Light::DirectionalLight:
            removeObjectFromList(dirLights, light);
            break;
        case Light::PointLight:
            removeObjectFromList(pointLights, light);
            break;
        case Light::SpotLight:
            removeObjectFromList(spotLights, light);
            break;
        }
    }

    //---------------------------------------------------------------------------
    //  EmptyScene - Init
    //---------------------------------------------------------------------------

    void EmptyScene::init(RenderingEngine* renderer)
    {
        Camera* cam = new Camera(Transform(Point3f(0, 0, 20)));
        renderer->setCamera(cam);
    }


}