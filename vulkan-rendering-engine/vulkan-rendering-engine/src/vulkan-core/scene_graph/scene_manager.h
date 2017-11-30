#ifndef SCENE_MANAGER_H_
#define SCENE_MANAGER_H_

#include "scene.h"
#include <map>

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define CUR_SCENE                   SceneManager::getCurrentScene()

    //---------------------------------------------------------------------------
    //  SceneManager Class
    //---------------------------------------------------------------------------

    class SceneManager
    {
        friend class VulkanBase;        // access to init() & destroy()
        friend class RenderingEngine;   // access to update()

        static void init(RenderingEngine* renderer);    // Initialize the startup-scene
        static void update(float delta);                // Updated by the rendering-engine for now
        static void destroy();                          // Called in destructor of the rendering-engine -> replace through subsystem

    public:
        static Scene* getCurrentScene(){ return currentScene; }

        static void switchScene(Scene* newScene, bool deleteOldScene = true);
        static void switchScene(const std::string& sceneName, bool deleteOldScene = true);

        static bool deleteScene(const std::string& sceneName);
        static Scene* getScene(const std::string& sceneName);
        static const std::map<std::string, Scene*>& getScenes(){ return scenes; }

    private:
        // forbid copy and copy assignment
        SceneManager(const SceneManager& sm) = delete;
        SceneManager& operator=(const SceneManager& sm) = delete;

        static std::map<std::string, Scene*> scenes;
        static Scene* currentScene;

        static void switchToNewScene();
        static void load(Scene* scene);
        static void unload(Scene* scene);
    };


}


#endif // !SCENE_MANAGER_H_
