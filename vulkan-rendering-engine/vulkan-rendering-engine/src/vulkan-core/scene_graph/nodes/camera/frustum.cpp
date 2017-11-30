#include "frustum.h"

#include "vulkan-core/scene_graph/nodes/components/colliders/sphere_collider.h"
#include "vulkan-core/scene_graph/nodes/camera/camera.h"
#include "vulkan-core/scene_graph/scene_graph.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Update this frustum (e.g. update culling planes, calculate vertex-positions of the frustum in world-space
    void Frustum::update(const Mat4f& viewProjection)
    {
        updateCullingPlanes(viewProjection);
        calculateVertices();
    }

    // Check if a sphere is in the view frustum
    bool Frustum::checkSphere(const Vec3f& pos, float radius) const
    {
        for (unsigned int i = 0; i < planes.size(); i++)
        {
            if ((planes[i].x() * pos.x()) + (planes[i].y() * pos.y()) + (planes[i].z() * pos.z()) + planes[i].w() <= -radius)
            {
                return false;
            }
        }
        return true;
    }

    // Check if a sphere is in the view frustum plane
    bool Frustum::checkSphere(SphereCollider* sphereCollider) const
    {
        return checkSphere(sphereCollider->getWorldPos(), sphereCollider->getRadius());
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Update the view-frustum culling planes
    void Frustum::updateCullingPlanes(const Mat4f& viewProjection)
    {
        planes[LEFT].x() = viewProjection[0].w() + viewProjection[0].x();
        planes[LEFT].y() = viewProjection[1].w() + viewProjection[1].x();
        planes[LEFT].z() = viewProjection[2].w() + viewProjection[2].x();
        planes[LEFT].w() = viewProjection[3].w() + viewProjection[3].x();

        planes[RIGHT].x() = viewProjection[0].w() - viewProjection[0].x();
        planes[RIGHT].y() = viewProjection[1].w() - viewProjection[1].x();
        planes[RIGHT].z() = viewProjection[2].w() - viewProjection[2].x();
        planes[RIGHT].w() = viewProjection[3].w() - viewProjection[3].x();

        planes[TOP].x() = viewProjection[0].w() - viewProjection[0].y();
        planes[TOP].y() = viewProjection[1].w() - viewProjection[1].y();
        planes[TOP].z() = viewProjection[2].w() - viewProjection[2].y();
        planes[TOP].w() = viewProjection[3].w() - viewProjection[3].y();

        planes[BOTTOM].x() = viewProjection[0].w() + viewProjection[0].y();
        planes[BOTTOM].y() = viewProjection[1].w() + viewProjection[1].y();
        planes[BOTTOM].z() = viewProjection[2].w() + viewProjection[2].y();
        planes[BOTTOM].w() = viewProjection[3].w() + viewProjection[3].y();

        planes[BACK].x() = viewProjection[0].w() + viewProjection[0].z();
        planes[BACK].y() = viewProjection[1].w() + viewProjection[1].z();
        planes[BACK].z() = viewProjection[2].w() + viewProjection[2].z();
        planes[BACK].w() = viewProjection[3].w() + viewProjection[3].z();

        planes[FRONT].x() = viewProjection[0].w() - viewProjection[0].z();
        planes[FRONT].y() = viewProjection[1].w() - viewProjection[1].z();
        planes[FRONT].z() = viewProjection[2].w() - viewProjection[2].z();
        planes[FRONT].w() = viewProjection[3].w() - viewProjection[3].z();

        for (unsigned int i = 0; i < planes.size(); i++)
        {
            float length = sqrtf(planes[i].x() * planes[i].x() + planes[i].y() * planes[i].y() + planes[i].z() * planes[i].z());
            planes[i] /= length;
        }
    }

    // Calculate the vertices of the view-frustum in world space
    void Frustum::calculateVertices()
    {
        Point3f camPos      = camera->getWorldPosition();
        Vec3f camForward    = camera->getWorldRotation().getForward();

        Vec3f camUp         = camera->getWorldRotation().getUp();
        Vec3f camRight      = camera->getWorldRotation().getRight();

        Point3f nearCenter    = camPos + camForward * camera->getZNear();
        Point3f farCenter     = camPos + camForward * camera->getZFar();

        switch (camera->getMode())
        {
        case Camera::ORTHOGRAPHIC:
            vertices[FAR_TOP_LEFT]      = farCenter + camUp * camera->getTop() + camRight * camera->getLeft();
            vertices[FAR_TOP_RIGHT]     = farCenter + camUp * camera->getTop() + camRight * camera->getRight();
            vertices[FAR_BOTTOM_LEFT]   = farCenter + camUp * camera->getBottom() + camRight * camera->getLeft();
            vertices[FAR_BOTTOM_RIGHT]  = farCenter + camUp * camera->getBottom() + camRight * camera->getRight();

            vertices[NEAR_TOP_LEFT]     = nearCenter + camUp * camera->getTop() + camRight * camera->getLeft();
            vertices[NEAR_TOP_RIGHT]    = nearCenter + camUp * camera->getTop() + camRight * camera->getRight();
            vertices[NEAR_BOTTOM_LEFT]  = nearCenter + camUp * camera->getBottom() + camRight * camera->getLeft();
            vertices[NEAR_BOTTOM_RIGHT] = nearCenter + camUp * camera->getBottom() + camRight * camera->getRight();
            break;
        case Camera::PERSPECTIVE:
            float fovRadians    = Mathf::deg2Rad(camera->getFOV());
            float nearHeight    = 2 * tan(fovRadians / 2) * camera->getZNear();
            float farHeight     = 2 * tan(fovRadians / 2) * camera->getZFar();
            float nearWidth     = nearHeight * camera->getAspectRatio();
            float farWidth      = farHeight * camera->getAspectRatio();

            vertices[FAR_TOP_LEFT]      = farCenter + camUp * (farHeight*0.5f) - camRight * (farWidth*0.5f);
            vertices[FAR_TOP_RIGHT]     = farCenter + camUp * (farHeight*0.5f) + camRight * (farWidth*0.5f);
            vertices[FAR_BOTTOM_LEFT]   = farCenter - camUp * (farHeight*0.5f) - camRight * (farWidth*0.5f);
            vertices[FAR_BOTTOM_RIGHT]  = farCenter - camUp * (farHeight*0.5f) + camRight * (farWidth*0.5f);

            vertices[NEAR_TOP_LEFT]     = nearCenter + camUp * (nearHeight*0.5f) - camRight * (nearWidth*0.5f);
            vertices[NEAR_TOP_RIGHT]    = nearCenter + camUp * (nearHeight*0.5f) + camRight * (nearWidth*0.5f);
            vertices[NEAR_BOTTOM_LEFT]  = nearCenter - camUp * (nearHeight*0.5f) - camRight * (nearWidth*0.5f);
            vertices[NEAR_BOTTOM_RIGHT] = nearCenter - camUp * (nearHeight*0.5f) + camRight * (nearWidth*0.5f);
            break;
        }

        Point3f sum;
        for (unsigned int i = 0; i < vertices.size(); i++)
            sum += vertices[i];

        center = static_cast<Point3f>(sum / static_cast<float>(vertices.size()));
    }


}

