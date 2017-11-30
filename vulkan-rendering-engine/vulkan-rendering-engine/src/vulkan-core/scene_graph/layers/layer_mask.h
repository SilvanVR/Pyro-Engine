#ifndef LAYER_MASK_H_
#define LAYER_MASK_H_

#include <vector>
#include <string>

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define LAYER_DEFAULT                   "Default"
    #define LAYER_UI                        "UI"
    #define LAYER_LIGHTS                    "Lights"
    #define LAYER_IGNORE_RENDERING          "IgnoreRendering"
    #define LAYER_IGNORE_SHADOW_RENDERING   "IgnoreShadowRendering"
    #define LAYER_IGNORE_RAYCASTS           "IgnoreRaycasts"
    #define LAYER_BOUNDING_BOX              "BoundingBox"

    #define LAYER_ALL_3D    LAYER_DEFAULT,LAYER_LIGHTS,LAYER_IGNORE_SHADOW_RENDERING
    #define LAYER_ALL       LAYER_DEFAULT,LAYER_UI,LAYER_LIGHTS,LAYER_IGNORE_RENDERING,LAYER_IGNORE_SHADOW_RENDERING,LAYER_IGNORE_RAYCASTS,LAYER_BOUNDING_BOX

    //---------------------------------------------------------------------------
    //  LayerMask-Class
    //---------------------------------------------------------------------------

    class LayerMask
    {
    public:
        LayerMask(const std::vector<std::string>& layerNames = {});
        ~LayerMask() {}

        bool check(const LayerMask& other) { return *this & other; }

        // Add a layer to this layer-mask
        void addLayer(const std::string& name);

        // Remove a layer from this layer-mask
        void removeLayer(const std::string& name);

        // Change this layer-mask to the given one. (It removes all layers and set the layer to the given one)
        void changeLayer(const std::string& name);

        // Change this layer-mask to the given ones. (It removes all layers and set the layer to the given ones)
        void changeLayer(const std::vector<std::string>& names);

        // Check if this layer-mask contains zero layer
        bool empty() { return layerMask == 0; }

        bool operator&(const LayerMask& other) const { return (this->layerMask & other.layerMask) != 0; }

    private:
        int layerMask;
    };
    



}



#endif // !LAYER_MASK_H_
