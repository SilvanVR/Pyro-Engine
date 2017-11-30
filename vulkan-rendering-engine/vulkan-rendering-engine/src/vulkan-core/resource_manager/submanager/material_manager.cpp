#include "material_manager.h"

#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/scene_graph/scene_manager.h"

namespace Pyro
{

    static ResourceID defaultMaterialID = 0;

    //---------------------------------------------------------------------------
    //  ModelManager - Init() 
    //---------------------------------------------------------------------------

    void MaterialManager::init()
    {
        // Depends on the G-Buffer shader
        auto defaultMat = PBRMATERIAL({ TEXTURE_GET(TEX_DEFAULT), MATERIAL_DEFAULT });
        defaultMaterialID = defaultMat.getID();
        addGlobalResource(defaultMat);
        addGlobalResource( MATERIAL(SHADER(SHADER_FW_WIREFRAME), MATERIAL_BOUNDING_BOX) );
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    ResourceID MaterialManager::createMaterial(ShaderPtr shader, const std::string& name)
    {
        Material* pMaterial = new Material(shader, name);
        pMaterial->setBoundScene(SceneManager::getCurrentScene());
        return addToResourceTable(pMaterial);
    }

    ResourceID MaterialManager::createPBRMaterial(const PBRMaterialParams& params)
    {
        PBRMaterial* pMaterial = new PBRMaterial(params);
        pMaterial->setBoundScene(SceneManager::getCurrentScene());
        return addToResourceTable(pMaterial);
    }

    ResourceID MaterialManager::get(const std::string& name)
    {
        ResourceID id = m_resourceTable.find([&](Material* pMat) -> bool {
            return pMat->getName() == name;
        });

        if (id == RESOURCE_ID_INVALID)
        {
            Logger::Log ("MaterialManager::get(): Could not find a material with name '" + name + "'. "
                         "Returning default one instead.", LOGTYPE_WARNING);
            return getDefaultMaterial();
        }
        return id;
    }

    ResourceID MaterialManager::getDefaultMaterial() 
    { 
        return defaultMaterialID;
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    ResourceID MaterialManager::addToResourceTable(Material* material)
    {
        ResourceID id = m_resourceTable.add(material);
        Logger::Log("Map material '" + material->getName() + "' to ID #" + TS(id), LOGTYPE_INFO, LOG_LEVEL_NOT_IMPORTANT);
        return id;
    }

}