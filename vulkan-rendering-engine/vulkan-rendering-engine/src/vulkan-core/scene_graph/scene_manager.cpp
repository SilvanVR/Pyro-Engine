#include "scene_manager.h"

#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"
#include "vulkan-core/rendering_engine.h"
#include "logger/logger.h"

namespace Pyro
{

    static RenderingEngine*     renderer;               // reference to the renderer
    static Scene*               sceneToLoad = nullptr;  // If this is not nullptr the Manager will switch to this scene when possible
    static bool                 deleteOldScene = true;  // If this is true when the manager switches scene it will delete the old one

    //---------------------------------------------------------------------------
    //  Static Variables
    //---------------------------------------------------------------------------

    std::map<std::string, Scene*>   SceneManager::scenes;
    Scene*                          SceneManager::currentScene = nullptr;

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void SceneManager::update(float delta)
    {
        if(sceneToLoad) switchToNewScene();
        currentScene->updateScene(delta);
        currentScene->lateUpdate(delta);
    }

    Scene* SceneManager::getScene(const std::string& sceneName)
    {
        bool sceneExists = scenes.count(sceneName) == 1;
        return sceneExists ? scenes[sceneName] : nullptr;
    }

    bool SceneManager::deleteScene(const std::string& sceneName)
    {
        bool sceneExists = scenes.count(sceneName) == 1;
        if (sceneExists)
        {
            Scene* sceneToDelete = scenes[sceneName];
            scenes.erase(sceneName);
            delete sceneToDelete;
            return true;
        }
        return false;
    }

    void SceneManager::switchScene(Scene* newScene, bool _deleteOldScene)
    {
        deleteOldScene = _deleteOldScene;
        sceneToLoad = newScene;
    }

    void SceneManager::switchScene(const std::string& sceneName, bool deleteOldScene)
    {
        if (scenes.count(sceneName) == 0)
        {
            Logger::Log("Scene with name '" + sceneName + "' does not exist. Can't switch to it.", LOGTYPE_WARNING);
            return;
        }

        switchScene(scenes[sceneName], deleteOldScene);
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    void SceneManager::init(RenderingEngine* _renderer)
    {
        renderer = _renderer;
        sceneToLoad = new EmptyScene();
        switchToNewScene();
    }

    void SceneManager::destroy()
    {
        // delete all remaining scenes
        for (auto& pair : scenes)
        {
            currentScene = pair.second;
            delete pair.second;
        }
        currentScene = nullptr;
    }

    void SceneManager::switchToNewScene()
    {
        if (currentScene != nullptr)
        {
            currentScene->onCurrentSceneUnload();
            currentScene->transferGlobalObjects(sceneToLoad);
        }

        if (deleteOldScene)
        {
            unload(currentScene);
            currentScene = nullptr;
        }
        renderer->resetStateToDefault();
        load(sceneToLoad);

        Logger::Log("Done loading scene '" + sceneToLoad->getName() + "'.", LOGTYPE_INFO);
        sceneToLoad = nullptr;
    }

    void SceneManager::unload(Scene* scene)
    {
        vkDeviceWaitIdle(VulkanBase::getDevice());
        if (scene != nullptr)
        {
            const std::string& sceneName = scene->getName();
            Logger::Log("Unload scene '" + sceneName + "'", LOGTYPE_INFO);
            scenes.erase(scenes.find(sceneName));
            delete scene;
        }
    }

    void SceneManager::load(Scene* newScene)
    {
        std::string sceneName = newScene->getName();
        Logger::Log("Load scene '" + sceneName + "' ...", LOGTYPE_INFO);

        // Order dependant. The currentScene variable has to be set before "init()/reload()" is called.
        currentScene = newScene;
        bool sameScene = false;
        if (scenes.count(sceneName) != 0)
        {
            sameScene = newScene == scenes[sceneName];
            if (sameScene)
            {
                Logger::Log("Scene with name '" + sceneName + "' already exists. Try to reload it.", LOGTYPE_INFO);
                scenes[sceneName]->reload(renderer);
            }
            else
            {
                Logger::Log("New scene with name '" + sceneName + "' was added, but a scene with the same name already exists. "
                            "Replacing the old one by the new one.", LOGTYPE_WARNING);
                delete scenes[sceneName];
            }
        }

        if (!sameScene)
        {
            scenes[sceneName] = newScene;
            newScene->init(renderer);
        }

        newScene->onCurrentSceneLoad(newScene);
    }

}