#include "cube.h"

#include "vulkan-core/resource_manager/resource_manager.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Static Members
    //---------------------------------------------------------------------------

    // Number of cubes in the scene
    uint32_t Cube::refCount = 0;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Cube::Cube(MaterialPtr material, const Transform& transform)
        : Renderable(std::string("Cube") + std::to_string(Cube::refCount), MESH(MODEL_CUBE_DEFAULT), material, transform)
    {
        Cube::refCount++;
    }
    
    Cube::Cube(const Transform& transform)
        : Cube(nullptr, transform)
    {}

    Cube::Cube(Point3f pos)
        : Cube(Transform(pos))
    {}


    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    //Delete the cube mesh if the last cube is deleted
    Cube::~Cube()
    {
        --Cube::refCount;
    }


}