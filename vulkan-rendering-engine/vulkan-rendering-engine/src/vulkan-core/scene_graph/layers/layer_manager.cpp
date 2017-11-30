#include "layer_manager.h"

#include <algorithm>
#include "logger/logger.h"
#include "layer_mask.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    LayerManager LayerManager::INSTANCE;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    LayerManager::LayerManager()
    {
        for(int i = 0; i < MAX_NUM_LAYERS; i++)
            layers[i] = LAYER_NOT_USED;
    }

    //---------------------------------------------------------------------------
    //  Init() & Destroy()
    //---------------------------------------------------------------------------

    void LayerManager::init()
    {
        addLayer(LAYER_DEFAULT, 0);
        addLayer(LAYER_UI, 1);
        addLayer(LAYER_LIGHTS, 2);
        addLayer(LAYER_IGNORE_RENDERING, 3);
        addLayer(LAYER_IGNORE_SHADOW_RENDERING, 4);
        addLayer(LAYER_IGNORE_RAYCASTS, 5);
        addLayer(LAYER_BOUNDING_BOX, 6);
    }

    void LayerManager::destroy()
    {
        // Nothing to do here yet..
    }

    //---------------------------------------------------------------------------
    //  Public Functions
    //---------------------------------------------------------------------------
    void LayerManager::addLayer(const std::string& name, int index)
    {
        if(index >= MAX_NUM_LAYERS)
            Logger::Log("LayerManager::addLayer(): Can't add Layer '" + name + "' with index #" + std::to_string(index) + 
                        ". Only " + std::to_string(MAX_NUM_LAYERS) + " are allowed.", LOGTYPE_ERROR);

        // Check if index and/or name is already used
        INSTANCE.checkIfLayerAlreadyExists(name, index);

        INSTANCE.layers[index] = name;
    }

    void LayerManager::removeLayer(const std::string& name)
    {
        for (int i = 0; i < MAX_NUM_LAYERS; i++)
            if(INSTANCE.layers[i] == name)
                INSTANCE.layers[i] == LAYER_NOT_USED;
    }

    int LayerManager::getLayer(const std::string& name)
    {
        for (int i = 0; i < MAX_NUM_LAYERS; i++)
            if (INSTANCE.layers[i] == name)
                return i;

        Logger::Log("LayerManager::getLayer(): Layer '" + name + "' not found!", LOGTYPE_WARNING);
        return LAYER_NOT_FOUND;
    }

    int LayerManager::getLayerMask(const std::vector<std::string>& layerNames)
    {
        int layerMask = 0;
        for (auto& layer : layerNames)
            layerMask |= (1 << getLayer(layer));
        return layerMask;
    }

    //---------------------------------------------------------------------------
    //  Private Functions
    //---------------------------------------------------------------------------

    void LayerManager::checkIfLayerAlreadyExists(const std::string& name, int index)
    {
        if(INSTANCE.layers[index] != LAYER_NOT_USED)
            Logger::Log("LayerManager::addLayer(): Given Layer-Index #" + std::to_string(index) + 
                        " is already used by Layer '" + INSTANCE.layers[index] + "'", LOGTYPE_ERROR);

        for (int i = 0; i < MAX_NUM_LAYERS; i++)
            if (INSTANCE.layers[i] == name)
                Logger::Log("LayerManager::addLayer(): Given Layer-Name '" + name + 
                            "' already exists at index #" + std::to_string(i), LOGTYPE_ERROR);
    }



}