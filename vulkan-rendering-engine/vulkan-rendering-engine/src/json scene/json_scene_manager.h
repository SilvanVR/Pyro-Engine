#ifndef JSON_SCENE_MANAGER_H_
#define JSON_SCENE_MANAGER_H_

#include "json_scene.h"

// Intent: Do not load a scene from a json request again

// This class manages the switching between several json scenes. 
// If a json-request comes in it checks if the corresponding scene 
// still exists and just change what has changed in the json-data.
// If the request is new it creates a new json scene.

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Enums
    //---------------------------------------------------------------------------

    enum class ECleanupStrategy
    {
        NONE,
        TIMER,                      // Scene will be deleted after a specified amount of time.
        FIXED_AMOUNT_OF_SCENES,     // Keep always N numbers of scenes. Least Recently Used cleanup strategy.
        FULL_UTILIZATION,           // Keep as much scenes as possible. Least Recently Used cleanup strategy.
    };

    //---------------------------------------------------------------------------
    //  JSONSceneManager class
    //---------------------------------------------------------------------------

    class JSONSceneManager
    {
    public:
        // Switch to another scene which will be rendered with consecutive calls to draw()
        // @jsonText: json-string 
        static void switchScene(const std::string& jsonText);


        // Switch to another scene which will be rendered with consecutive calls to draw()
        // @virtualPath: virtual/relative/absolute path to the .json file
        static void switchSceneFromFile(const std::string& virtualPath);


        // Load a .json from a file and create a JSON object for it
        // @virtualPath: virtual/relative/absolute path to the .json file
        static JSON loadFromFile(const std::string& virtualPath);


        // Set the cleanup strategy for the loaded json scenes (when they will be removed).
        // @strategy:   The strategy to use
        // @param:      params for the corresponding strategy:
        //              TIMER:                  Scenes will be kept in memory as long as this time in seconds
        //              FIXED_AMOUNT_OF_SCENES: Number of scenes to keep in memory
        //              FULL_UTILIZATION:       Param does nothing
        static void setCleanupStrategy(ECleanupStrategy strategy, float param = 0.0f);


        // Determines how often the cleanup callback will be called.
        // @ newInterval: Time in which the cleanup callback will be called every "newInterval" MILLISECONDS
        static void setCleanupCallbackInterval(uint64_t newInterval){ cleanupCallbackInterval = newInterval; }


        // Checks periodiacally if given json files are still up to date. If not they will be reloaded automatically.
        // @enabled: Enable or disable Hot-Reloading
        // @interval: How often it will check the files in SECONDS
        static void setHotReloading(bool enabled, float interval = 0.5f);

        // Misc
        static uint64_t getCleanupCallbackInterval(){ return cleanupCallbackInterval; }
        static std::vector<JSONScene*> getJSONScenes(){ return jsonScenes; }
        static uint32_t numJSONScenes(){ return static_cast<uint32_t>(jsonScenes.size()); }

    private:
        static std::vector<JSONScene*>  jsonScenes;
        static uint64_t                 cleanupCallbackInterval;
        static ECleanupStrategy         cleanupStrategy;

        // Perform some actions based on which cleanup strategy was chosen
        static void checkCleanupStrategy();


        // Creates a JSON object from a jsonString
        // @jsonString: actual json data
        static JSON parseJSON(const char* jsonString);


        // Return the name/id of the scene
        // @json: the json object in which it searches for the name
        static std::string  getSceneIdentifier(const JSON& json);


        // Checks if a scene is already loaded and return it if so, otherwise it constructs a new one.
        // @json: The json object representing the scene
        static JSONScene*   getScene(const JSON& json);


        // Creates a new scene from a json object. 
        // @sceneId:    name/id of the scene
        // @json:       json-object which describes the scene
        static JSONScene*   createNewScene(const std::string& sceneId, const JSON& json);


        // Modifies an existing scene 
        // @scene:  scene to modify
        // @json:   json-object which describes the modified scene
        static void         modifyExistingScene(JSONScene* scene, const JSON& json);
    };

}



#endif