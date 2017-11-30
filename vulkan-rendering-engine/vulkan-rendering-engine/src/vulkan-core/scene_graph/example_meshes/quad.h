#ifndef QUAD_H_
#define QUAD_H_

#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Cube class
    //---------------------------------------------------------------------------

    class Quad : public Renderable
    {

    public:
        Quad(MaterialPtr material, const Transform& transform = Transform());
        Quad(const Transform& transform);
        Quad(const std::string& name, MaterialPtr material, const Transform& transform = Transform());
        Quad(Point3f pos);
        ~Quad();

    private:
        // forbid copy and copy assignment
        Quad(const Quad& quad);
        Quad& operator=(const Quad& quad) {};

        // How much quads are in the scene
        static uint32_t refCount;
    };

}

#endif // !QUAD_H_
