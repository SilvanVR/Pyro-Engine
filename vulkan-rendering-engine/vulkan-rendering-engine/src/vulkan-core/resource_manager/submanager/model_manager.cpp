#include "model_manager.h"

#include "vulkan-core/resource_manager/mesh_loading/assimp_loader.h"
#include "vulkan-core/resource_manager/resource_manager.h"
#include "file_system/file_system.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  ModelManager - Init() & Destroy()
    //---------------------------------------------------------------------------

    void ModelManager::init()
    {
        addGlobalResource( MESH(MODEL_QUAD_DEFAULT) );
        addGlobalResource( MESH(MODEL_CUBE_DEFAULT) );
        addGlobalResource( MESH(MODEL_SPHERE_DEFAULT) );
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void ModelManager::updateResources()
    {
        int visitedMeshes = 0;
        for (ResourceID i = 0; i < m_resourceTable.maxPossibleResources(); i++)
        {
            Mesh* res = m_resourceTable[i];
            if (res != nullptr)
            {
                const std::string& filePath = res->getFilePath();
                Logger::Log("   >>>>> " + filePath);

                if (!FileSystem::fileExists(filePath))
                    continue; // File no longer exists

                const SystemTime& fileTime = res->getFileTime();
                SystemTime curFileTime;
                bool success = FileSystem::getLastWrittenFileTime(filePath, curFileTime);
                if (fileTime != curFileTime)
                {
                    Logger::Log("Reloading resource " + filePath);
                    Mesh* pNewMesh = loadFromDisk(filePath);
                    m_resourceTable.exchangeData(i, pNewMesh);
                }

                // Quit loop if we checked all possible resources
                visitedMeshes++;
                if (visitedMeshes == m_resourceTable.getAmountOfResources())
                    break;
            }
        }
    }

    ResourceID ModelManager::createMesh(const std::string& filepath)
    {
        ResourceID meshID = m_resourceTable.find([&](Mesh* mesh) -> bool {
            return mesh->getFilePath() == filepath;
        });

        if (meshID == RESOURCE_ID_INVALID)
        {
            Mesh* pMesh = loadFromDisk(filepath);
            meshID = addToResourceTable(pMesh);
        }

        return meshID;
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    ResourceID ModelManager::addToResourceTable(Mesh* mesh)
    {
        ResourceID id = m_resourceTable.add(mesh);
        Logger::Log("Map mesh '" + mesh->getFilePath() + "' to ID #" + TS(id), LOGTYPE_INFO, LOG_LEVEL_NOT_IMPORTANT);
        return id;
    }

    Mesh* ModelManager::loadFromDisk(const std::string& filepath)
    {
        Mesh* pMesh = nullptr;
        std::string fileExtension = FileSystem::getFileExtension(filepath);
        if (fileExtension == "dae")
        {
            Logger::Log("Loading Collada-Model with pre-Transformed Vertices '" + filepath + "'");
            pMesh = AssimpLoader::loadMesh(filepath, true);
        }
        else
        {
            Logger::Log("Loading Model '" + filepath + "'");
            pMesh = AssimpLoader::loadMesh(filepath, false);
        }
        return pMesh;
    }

}