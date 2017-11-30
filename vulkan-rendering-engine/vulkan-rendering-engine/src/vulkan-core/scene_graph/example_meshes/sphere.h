#ifndef SPHERE_H_
#define SPHERE_H_

#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Cube class
    //---------------------------------------------------------------------------

    class Sphere : public Renderable
    {

    public:
        Sphere(MaterialPtr material, const Transform& transform = Transform());
        Sphere(const Transform& transform);
        Sphere(const std::string& name, MaterialPtr material, const Transform& transform = Transform());
        Sphere(Point3f pos);
        ~Sphere();

    private:
        // forbid copy and copy assignment
        Sphere(const Sphere& s) = delete;
        Sphere& operator=(const Sphere& s) = delete;

        // How much spheres are in the scene
        static uint32_t refCount;
    };

}

#endif // !SPHERE_H_
