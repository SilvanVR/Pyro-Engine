#include "sphere_collider.h"

#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    SphereCollider::SphereCollider(MeshPtr mesh)
       : SphereCollider(mesh->getDimension().maxRadius)
    {}

    SphereCollider::SphereCollider(float _radius)
        : radius(_radius), meshRadius(MESH(MODEL_SPHERE_DEFAULT)->getDimension().max.maxValue())
    {
        boundingBox = new Renderable("BoundingBox", MESH(MODEL_SPHERE_DEFAULT), 
                                                    MATERIAL_GET(MATERIAL_BOUNDING_BOX),
                                                    Transform(), Node::Dynamic, false);
        boundingBox->changeLayer({ LAYER_IGNORE_SHADOW_RENDERING, LAYER_IGNORE_RAYCASTS, LAYER_BOUNDING_BOX });
        boundingBox->setIsActive(false);
    }

    void SphereCollider::update(float delta)
    {
        if(!boundingBox->isActive()) return;
        float scale = getRadius() / meshRadius;
        boundingBox->getTransform().scale = Vec3f(scale, scale, scale);
        boundingBox->getTransform().position = parentNode->getWorldPosition();
        boundingBox->getTransform().rotation = parentNode->getWorldRotation();
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    SphereCollider::~SphereCollider()
    {
        delete boundingBox;
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Check if the given ray intersects this collider
    HitInfo SphereCollider::intersects(const Ray& ray)
    {
        HitInfo hitInfo = Ray::HIT_NOTHING;

        const Point3f& rayPos = ray.getOrigin();
        const Vec3f& rayDir = ray.getDirection();

        float sphereRadius = getRadius();

        // Vector from "rayPos" to center of the collider
        Vec3f L = getWorldPos() - rayPos;

        // Project vector "L" onto the rays direction
        float d = L.dot(rayDir);

        // Projection of vector "L" onto the "ray" is negative, which means the object must be behind "rayPos"
        //if (d < 0)
        //    return hitInfo;

        // Calculate nearest distance from the ray to the center using the pythagoras theorem
        float lMagnitude = L.magnitude();
        float nearestDistanceRayToCenter = sqrt(lMagnitude * lMagnitude - d * d);

        // If the distance is greater than the sphere's radius its completely outside
        if (nearestDistanceRayToCenter > sphereRadius)
            return hitInfo;

        // I dont know how to describe this vector. Look at http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
        float tHC = sqrt(sphereRadius * sphereRadius - nearestDistanceRayToCenter * nearestDistanceRayToCenter);

        float t0 = d - tHC;
        float t1 = d + tHC;

        if (t0 > t1) std::swap(t0, t1);

        if (t0 < 0) {
            t0 = t1; // if t0 is negative, let's use t1 instead
            if (t0 < 0) return hitInfo; // both t0 and t1 are negative 
        }

        // Calculate hit-point
        Vec3f hitPoint = rayPos + rayDir * t0;

        // Check if hit-point is outside of the ray's range
        if ((rayPos - hitPoint).magnitude() > ray.getDistance())
            return hitInfo;

        // Hitpoint is within range, set position in hitInfo and return it
        hitInfo.pos  = rayPos + rayDir * t0;
        hitInfo.node = getParent();

        return hitInfo;
    }

}
