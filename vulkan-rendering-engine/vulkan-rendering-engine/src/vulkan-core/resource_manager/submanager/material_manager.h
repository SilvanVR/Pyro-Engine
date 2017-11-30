#ifndef MATERIAL_MANAGER_H_
#define MATERIAL_MANAGER_H_

#include "vulkan-core/data/material/pbr_material.h"
#include "i_resource_submanager.hpp"
#include <vector>

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define MATERIAL_DEFAULT        "DefaultQuad"
    #define MATERIAL_BOUNDING_BOX   "BoundingBoxMaterial"

    //---------------------------------------------------------------------------
    //  ModelManager Class
    //---------------------------------------------------------------------------

    class MaterialManager : public IResourceSubManager<Material>
    {
    public:
        MaterialManager() {}
        ~MaterialManager() {}

        ResourceID get(const std::string& name);
        ResourceID createMaterial(ShaderPtr shader, const std::string& name);
        ResourceID createPBRMaterial(const PBRMaterialParams& params);
        ResourceID getDefaultMaterial();

        // IResourceSubManager Interface
        void init() override;

    private:
        ResourceID addToResourceTable(Material* material) override;
    };

}



#endif // !MATERIAL_MANAGER_H_
