#include "layer_mask.h"

#include "layer_manager.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    LayerMask::LayerMask(const std::vector<std::string>& layerNames)
    {
        layerMask = LayerManager::getLayerMask(layerNames);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void LayerMask::addLayer(const std::string& name)
    {
        layerMask |= (1 << LayerManager::getLayer(name));
    }

    void LayerMask::removeLayer(const std::string& name)
    {
        layerMask &= ~(1 << LayerManager::getLayer(name));
    }

    void LayerMask::changeLayer(const std::string& name)
    {
        layerMask = (1 << LayerManager::getLayer(name));
    }

    void LayerMask::changeLayer(const std::vector<std::string>& names)
    {
        layerMask = LayerManager::getLayerMask(names);
    }







}