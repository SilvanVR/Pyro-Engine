#ifndef FRUSTUM_H_
#define FRUSTUM_H_

#include "build_options.h"
#include "math/math_interface.h"
#include <array>

namespace Pyro
{

    class Camera;
    class SphereCollider;

    class Frustum
    {
        // adapt the culling of meshes and lights
        const float LIGHT_RADIUS_MODIFIER = 0.5f;

        // Viewfrustum - Culling parameters
        enum side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, BACK = 4, FRONT = 5 };
        std::array<Vec4f, 6> planes;

    public:
        Frustum(Camera* _camera) : camera(_camera) {}

        // Check if a sphere is in the view frustum plane
        bool checkSphere(const Vec3f& pos, float radius) const;

        // Check if a sphere is in the view frustum plane
        bool checkSphere(SphereCollider* sphereCollider) const;

        // Update this frustum (e.g. update culling planes, calculate vertex-positions of the frustum in world-space
        void update(const Mat4f& viewProjection);

        std::array<Point3f, 8>& getVertices(){ return vertices; }

    private:
        // The camera this frustum belongs to
        Camera* camera;

        // Viewfrustum - Vertices
        enum corner { NEAR_TOP_LEFT = 0, NEAR_TOP_RIGHT = 1, NEAR_BOTTOM_LEFT = 2, NEAR_BOTTOM_RIGHT = 3, 
                      FAR_TOP_LEFT = 4, FAR_TOP_RIGHT = 5, FAR_BOTTOM_LEFT = 6, FAR_BOTTOM_RIGHT = 7 };
        std::array<Point3f, 8> vertices;

        // Viewfrustum - center
        Point3f center;

        // Calculate the vertices of the view-frustum in world space
        void calculateVertices();

        // Updated when getViewProjection() in Camera-Class is called
        void updateCullingPlanes(const math::Mat4f& viewProjection);

    };

}

#endif // !FRUSTUM_H_
