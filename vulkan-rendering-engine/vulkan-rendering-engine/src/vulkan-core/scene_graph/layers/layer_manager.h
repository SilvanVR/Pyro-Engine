#ifndef LAYER_MANAGER_H_
#define LAYER_MANAGER_H_

#include <string>
#include <vector>

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define MAX_NUM_LAYERS                  32
    #define LAYER_NOT_USED                  ""
    #define LAYER_NOT_FOUND                 -1

    //---------------------------------------------------------------------------
    //  LayerManager Class
    //---------------------------------------------------------------------------

    class LayerManager
    {
        friend class VulkanBase; // Access to init() & destroy()
        friend class LayerMask; // Access to getLayerMask() & getLayer()

        static int getLayer(const std::string& name);
        static int getLayerMask(const std::vector<std::string>& layerNames);

    public:
        LayerManager();
        ~LayerManager() {}

        static void addLayer(const std::string& name, int index);
        static void removeLayer(const std::string& name);


    private:
        void checkIfLayerAlreadyExists(const std::string& name, int index);

        std::string layers[MAX_NUM_LAYERS];

        static void init();
        static void destroy();

        static LayerManager INSTANCE;
    };



}



#endif // !LAYER_MANAGER_H_

