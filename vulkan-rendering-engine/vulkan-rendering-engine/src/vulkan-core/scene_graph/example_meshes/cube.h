#ifndef CUBE_H_
#define CUBE_H_

#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Cube class
    //---------------------------------------------------------------------------

    class Cube : public Renderable
    {

    public:
        Cube(MaterialPtr material, const Transform& transform = Transform());
        Cube(const Transform& transform);
        Cube(Point3f pos);
        ~Cube();

    private:
        // forbid copy and copy assignment
        Cube(const Cube& cube);
        Cube& operator=(const Cube& cube) {};

        //How much meshes are in the scene
        static uint32_t refCount;
    };

}



#endif // !CUBE_H_