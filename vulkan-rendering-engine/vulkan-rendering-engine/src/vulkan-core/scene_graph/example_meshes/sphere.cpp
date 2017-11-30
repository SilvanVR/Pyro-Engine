#include "sphere.h"

#include "vulkan-core/resource_manager/resource_manager.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Static Members
    //---------------------------------------------------------------------------

    // Number of quads in the scene
    uint32_t Sphere::refCount = 0;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Sphere::Sphere(const std::string& name, MaterialPtr material, const Transform& transform)
        : Renderable(name, MESH(MODEL_SPHERE_DEFAULT), material, transform)
    {
        Sphere::refCount++;
    }

    Sphere::Sphere(MaterialPtr material, const Transform& transform)
        : Sphere(std::string("Sphere") + std::to_string(Sphere::refCount), material, transform)
    {}
    
    Sphere::Sphere(const Transform& transform)
        : Sphere(nullptr, transform)
    {}

    Sphere::Sphere(Point3f pos)
        : Sphere(Transform(pos))
    {}


    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Sphere::~Sphere()
    {
        --Sphere::refCount;
    }


}