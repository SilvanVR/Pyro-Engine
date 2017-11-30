#include "quad.h"

#include "vulkan-core/resource_manager/resource_manager.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Static Members
    //---------------------------------------------------------------------------

    // Number of quads in the scene
    uint32_t Quad::refCount = 0;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Quad::Quad(const std::string& name, MaterialPtr material, const Transform& transform)
        : Renderable(name, MESH(MODEL_QUAD_DEFAULT), material, transform)
    {
        Quad::refCount++;
    }

    Quad::Quad(MaterialPtr material, const Transform& transform)
        : Quad(std::string("Quad") + std::to_string(Quad::refCount), material, transform)
    {}
    
    Quad::Quad(const Transform& transform)
        : Quad(nullptr, transform)
    {}

    Quad::Quad(Point3f pos)
        : Quad(Transform(pos))
    {}


    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Quad::~Quad()
    {
        --Quad::refCount;
    }


}