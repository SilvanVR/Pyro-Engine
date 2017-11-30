#ifndef MODEL_MANAGER_H_
#define MODEL_MANAGER_H_

#include "vulkan-core/data/mesh/mesh.h"
#include "i_resource_submanager.hpp"

// This class manages behind the scenes all Meshes loaded during program execution

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define MODEL_QUAD_DEFAULT      "/models/plane.obj"
    #define MODEL_CUBE_DEFAULT      "/models/cube.obj"
    #define MODEL_SPHERE_DEFAULT    "/models/sphere.obj"

    //---------------------------------------------------------------------------
    //  ModelManager Class
    //---------------------------------------------------------------------------

    class ModelManager : public IResourceSubManager<Mesh>
    {
    public:
        ModelManager() {}
        ~ModelManager() {}

        // Checks if an Mesh object was already loaded and returns the ResourceID from it. If no mesh 
        // with the given filepath exists, it will be loaded and a new ResourceID will be returned.
        ResourceID createMesh(const std::string& filepath);

        // IResourceSubManager Interface
        void init() override;
        void updateResources() override;

    private:
        ResourceID addToResourceTable(Mesh* mesh) override;
        Mesh* loadFromDisk(const std::string& filePath);
    };








}

#endif // !MODEL_MANAGER_H_
