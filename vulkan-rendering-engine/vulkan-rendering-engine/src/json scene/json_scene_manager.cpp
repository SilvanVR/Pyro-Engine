#include "json_scene_manager.h"

#include "vulkan-core/memory_management/vulkan_memory_manager.h"
#include "vulkan-core/scene_graph/scene_manager.h"
#include "memory_manager/memory_manager.h"
#include "file_system/vfs.h"
#include "time/time.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines + Forward Declarations
    //---------------------------------------------------------------------------

    #define JSON_NAME_IDENTIFIER                "id"
    #define DEFAULT_CLEANUP_CALLBACK_INTERVAL   1000 // Every X-ms the cleanup-callback will be called
    #define ALMOST_OUT_OF_GPU_MEM_THRESHOLD     90 // in %
    #define ALMOST_OUT_OF_RAM_THRESHOLD         95 // in %

    void fullUtilizationCleanupStrategy();
    void timerCleanupStrategy();
    void deleteLeastRecentlyUsedScene();

    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    static struct CleanUpParams
    {
        float       maxLiveTime;
        uint32_t    maxScenes;
    } cleanupParams;

    struct JSONSceneInfo
    {
        float   liveTime;           // Livetime in milliseconds
        double  lastAccessTime;     // Last time the scene was active

        struct FileInformation
        {
            std::string filePath = "";   // File-Path of the json-file if it was a file
            SystemTime  fileTime = {};   // Last written time the file was modified
        } fileInfo;
        JSONSceneInfo() : liveTime(0), lastAccessTime(0), fileInfo() {}
    };
    static std::map<JSONScene*, JSONSceneInfo> additionalSceneInfo;

    //---------------------------------------------------------------------------
    //  Static Members
    //---------------------------------------------------------------------------

    std::vector<JSONScene*> JSONSceneManager::jsonScenes;
    uint64_t                JSONSceneManager::cleanupCallbackInterval = DEFAULT_CLEANUP_CALLBACK_INTERVAL;
    ECleanupStrategy        JSONSceneManager::cleanupStrategy         = ECleanupStrategy::NONE;

    //---------------------------------------------------------------------------
    //  Static Methods - Public
    //---------------------------------------------------------------------------

    void JSONSceneManager::setHotReloading(bool enabled, float interval)
    {
        static CallbackID callbackID = INVALID_CALLBACK_ID;
        Time::clearCallback(callbackID);

        if (enabled)
        {
            uint64_t intervalInSeconds = static_cast<uint64_t>(interval * 1000.0f);

            callbackID = Time::setInterval([] {
                Logger::Log("JSONSceneManager -- Hot Reloading: Checking if files are up to date...", LOGTYPE_INFO, LOG_LEVEL_NOT_SO_IMPORTANT);

                for (auto& pair : additionalSceneInfo)
                {
                    auto& scene = pair.first;
                    JSONSceneInfo& sceneInfo = pair.second;

                    const std::string& filePath = sceneInfo.fileInfo.filePath;
                    if (!FileSystem::fileExists(filePath))
                        continue; // File no longer exists

                    Logger::Log("  >>> " + scene->getName() + ": " + filePath, LOGTYPE_INFO, LOG_LEVEL_NOT_SO_IMPORTANT);

                    const SystemTime& fileTime = sceneInfo.fileInfo.fileTime;
                    SystemTime curFileTime;
                    bool success = FileSystem::getLastWrittenFileTime(filePath, curFileTime);
                    if (success && (fileTime != curFileTime))
                    {
                        Logger::Log("Reloading json file " + filePath);
                        switchSceneFromFile(filePath);
                    }
                }
            }, intervalInSeconds);
        }
    }

    void JSONSceneManager::setCleanupStrategy(ECleanupStrategy strategy, float param)
    {
        static CallbackID callbackID = INVALID_CALLBACK_ID;
        Time::clearCallback(callbackID);

        cleanupStrategy = strategy;
        switch (cleanupStrategy)
        {
        case ECleanupStrategy::TIMER:
            cleanupParams.maxLiveTime = param;
            Time::setInterval(timerCleanupStrategy, cleanupCallbackInterval);
            break;
        case ECleanupStrategy::FIXED_AMOUNT_OF_SCENES:
            if (param < 1)
            {
                Logger::Log("JSONSceneManager::setCleanupStragety(FIXED_AMOUNT_OF_SCENES, param): "
                            "Given param must be greater than 0, setting it to 1 then.", LOGTYPE_WARNING);
                cleanupParams.maxScenes = 1;
            } else {
                cleanupParams.maxScenes = static_cast<int>(param);
            }
            break;
        case ECleanupStrategy::FULL_UTILIZATION:
            break;
        case ECleanupStrategy::NONE:
            break;
        default:
            Logger::Log("JSONSceneManager::setCleanupStragety(): Unknown strategy. Scenes won't be deleted.", LOGTYPE_WARNING);
        }
    }

    void JSONSceneManager::switchSceneFromFile(const std::string& virtualPath)
    {
        std::string physicalPath = VFS::resolvePhysicalPath(virtualPath);
        JSON json = loadFromFile(physicalPath);

        if (json == nullptr)
            return;

        JSONScene* scene = getScene(json);

        additionalSceneInfo[scene].fileInfo.filePath = physicalPath;
        FileSystem::getLastWrittenFileTime(physicalPath, additionalSceneInfo[scene].fileInfo.fileTime);

        // Transition to scene
        SceneManager::switchScene(scene, false);
    }

    void JSONSceneManager::switchScene(const std::string& jsonText)
    {
        JSON json = parseJSON(jsonText.c_str());

        if (json == nullptr)
            return;

        JSONScene* scene = getScene(json);

        // Transition to scene
        SceneManager::switchScene(scene, false);
    }

    JSON JSONSceneManager::loadFromFile(const std::string& virtualPath)
    {
        std::string physicalPath = VFS::resolvePhysicalPath(virtualPath);
        if (!FileSystem::fileExists(physicalPath))
            Logger::Log("JSONScene::load(): File '" + physicalPath + "' does not exist.", LOGTYPE_ERROR);

        return parseJSON(FileSystem::load(physicalPath).c_str());
    }

    //---------------------------------------------------------------------------
    //  Static Methods - Private
    //---------------------------------------------------------------------------

    void JSONSceneManager::modifyExistingScene(JSONScene* scene, const JSON& json)
    {
        Logger::Log("JSONSceneManager: Recognized scene with id #" + scene->getName());

        // modify existing scene
        scene->modify(json);
    }

    JSONScene* JSONSceneManager::createNewScene(const std::string& sceneId, const JSON& json)
    {
        checkCleanupStrategy();

        Logger::Log("New scene #" + sceneId + " added!");
        JSONScene* jsonScene = new JSONScene(sceneId, json);

        jsonScene->onDelete([] (JSONScene* jsonScene) {
            Logger::Log(" >>> Delete JSON Scene " + jsonScene->getName());
            for (auto it = jsonScenes.begin(); it != jsonScenes.end();)
            {
                if (*it == jsonScene)
                {
                    it = jsonScenes.erase(it);
                    additionalSceneInfo.erase(jsonScene);
                } else { 
                    it++; 
                }
            }
        });

        jsonScenes.push_back(jsonScene);

        return jsonScene;
    }

    JSONScene* JSONSceneManager::getScene(const JSON& json)
    {
        std::string sceneName = getSceneIdentifier(json);

        JSONScene* scene = nullptr;
        if (sceneName != JSON_SCENE_NO_IDENTIFIER)
        {
            scene = dynamic_cast<JSONScene*>(SceneManager::getScene(sceneName));
        }
        else
        {
            Logger::Log("Scene with no ID was added. "
                        "A scene need a the field 'id' in order to be kept in memory", LOGTYPE_WARNING);
        }

        if (scene != nullptr)
        {
            modifyExistingScene(scene, json);
        }
        else
        {
            scene = createNewScene(sceneName, json);
        }
        additionalSceneInfo[scene].lastAccessTime = Time::getTotalRunningTime();

        return scene;
    }

    std::string JSONSceneManager::getSceneIdentifier(const JSON& json)
    { 
        if(json.count(JSON_NAME_IDENTIFIER))
            return json[JSON_NAME_IDENTIFIER];
        else
            return JSON_SCENE_NO_IDENTIFIER;
    }

    JSON JSONSceneManager::parseJSON(const char* jsonString)
    {
        JSON json;
        try
        {
            json = JSON::parse(jsonString);
        }
        catch (...)
        {
            Logger::Log("An Error occured in json.hpp: INVALID JSON. Check your json file", LOGTYPE_WARNING);
            return nullptr;
        }
        return json;
    }

    void JSONSceneManager::checkCleanupStrategy()
    {
        switch (cleanupStrategy)
        {
        case ECleanupStrategy::TIMER:
            break;
        case ECleanupStrategy::FIXED_AMOUNT_OF_SCENES:
            if (jsonScenes.size() == cleanupParams.maxScenes)
                deleteLeastRecentlyUsedScene();
            break;
        case ECleanupStrategy::FULL_UTILIZATION:
            fullUtilizationCleanupStrategy();
            break;
        }
    }

    //---------------------------------------------------------------------------
    //  Cleanup Strategies
    //---------------------------------------------------------------------------

    void timerCleanupStrategy()
    {
        Logger::Log("Timer cleanup...");
        for (auto& jsonScene : JSONSceneManager::getJSONScenes())
        {
            additionalSceneInfo[jsonScene].liveTime += JSONSceneManager::getCleanupCallbackInterval() * 0.001f;
            Logger::Log(" >>> " + jsonScene->getName() + ": " + TS(additionalSceneInfo[jsonScene].liveTime));

            if (jsonScene == SceneManager::getCurrentScene())
                continue;

            if (additionalSceneInfo[jsonScene].liveTime >= cleanupParams.maxLiveTime)
            {
                SceneManager::deleteScene(jsonScene->getName());
            }
        }
    }

    void fullUtilizationCleanupStrategy()
    {
        // Check if enough resources are free, if not delete scenes until it is

        // Check available GPU memory
        GPUMemoryInfo gpuMemInfo = VMM::getMemoryInfo();
        bool almostOutOfGPUMemory = gpuMemInfo.percentageUsed > ALMOST_OUT_OF_GPU_MEM_THRESHOLD;

        // Check available system RAM
        SystemMemoryInfo systemMemInfo = MemoryManager::getSystemMemoryInfo();
        bool almostOutOfRAM = systemMemInfo.percentageUsed > ALMOST_OUT_OF_RAM_THRESHOLD;

        while (almostOutOfRAM || almostOutOfGPUMemory)
        {
            if (JSONSceneManager::numJSONScenes() <= 1)
                return;

            deleteLeastRecentlyUsedScene();

            almostOutOfGPUMemory = VMM::getMemoryInfo().percentageUsed > ALMOST_OUT_OF_GPU_MEM_THRESHOLD;
            almostOutOfRAM = MemoryManager::getSystemMemoryInfo().percentageUsed > ALMOST_OUT_OF_RAM_THRESHOLD;
        }
    }

    void deleteLeastRecentlyUsedScene()
    {
        auto jsonScenes = JSONSceneManager::getJSONScenes();

        // Sort by last access time
        std::sort(jsonScenes.begin(), jsonScenes.end(), [](JSONScene* first, JSONScene* second) {
            return additionalSceneInfo[first].lastAccessTime < additionalSceneInfo[second].lastAccessTime;
        });

        // Delete least recently used scene
        JSONScene* sceneToDelete = jsonScenes.front();
        SceneManager::deleteScene(sceneToDelete->getName());
    }

}