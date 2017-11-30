#ifndef SPHERE_COLLIDER_H_
#define SPHERE_COLLIDER_H_

#include "../component.h"
#include "vulkan-core/mouse_picker/ray.h"
#include "vulkan-core/resource_manager/resource_manager.h"

namespace Pyro
{

    class Renderable;

    //---------------------------------------------------------------------------
    //  SphereCollider Class
    //---------------------------------------------------------------------------

    class SphereCollider : public Component
    {

    public:
        SphereCollider(float _radius = 0.0f);
        // Create a sphere-collider from a mesh. Take the bounds from the "Dimension"-Object of the mesh
        SphereCollider(MeshPtr mesh);
        ~SphereCollider();

        void update(float delta) override;

        Vec3f           getWorldPos() const { return parentNode->getWorldPosition(); }

        float           getRadius() const { return parentNode->getWorldScale().maxValue() * this->radius; }
        void            setRadius(float newRadius){ this->radius = newRadius; }

        // Check if the given ray intersects this collider
        HitInfo         intersects(const Ray& ray);

    private:
        float radius;

        Renderable* boundingBox;
        float meshRadius = 0.0f;
    };



}




#endif // !SPHERE_COLLIDER_H_

