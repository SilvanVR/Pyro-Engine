#include "mouse_picker.h"

#include "vulkan-core/scene_graph/nodes/components/colliders/sphere_collider.h"
#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"
#include "vulkan-core/scene_graph/nodes/camera/camera.h"
#include "vulkan-core/scene_graph/scene_manager.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "vulkan-core/rendering_engine.h"
#include "vulkan-core/mouse_picker/ray.h"
#include "Input/input.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    MousePicker::MousePicker()
    {
    }

    //---------------------------------------------------------------------------
    //  Public  Methods
    //---------------------------------------------------------------------------

    // Calculates and return the current mouse-ray from the main-camera
    Ray MousePicker::getCurrentRay(float distance)
    { 
        return calculateMouseRay(RenderingEngine::getCamera(), distance); 
    }

    // Return a node hit by the currentRay, nullptr if none was hit
    HitInfo MousePicker::raycast(const Ray& ray, LayerMask layerMask)
    {
        float       currentClosestDistance = FLT_MAX;
        HitInfo     currentHitInfo = Ray::HIT_NOTHING;

        for (auto& renderable : SceneManager::getCurrentScene()->getAllRenderables())
        {
            // Check if the renderable has the layer and if so immediately continue
            if(renderable->getLayerMask() & layerMask)
                continue;

            HitInfo hitInfo = renderable->getComponent<SphereCollider>()->intersects(ray);

            if(hitInfo == Ray::HIT_NOTHING)
                continue;

            // Check if the sphere which was hit is closer than the last object hit
            float dist = (ray.getOrigin() - hitInfo.pos).magnitude();

            if (dist < currentClosestDistance)
            {
                currentHitInfo = hitInfo;
                currentClosestDistance = dist;
            }
        }

        // Return closest object 
        return currentHitInfo;
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Calculates the current mouse-ray in world-space
    Ray MousePicker::calculateMouseRay(Camera* camera, float distance)
    {
        const Vec2i& screenPos        = Input::getMousePos();
        const Mat4f inverseProjection = camera->getProjection().inversed();
        const Mat4f inverseView       = camera->getViewMatrix().inversed();

        Vec3f worldSpace = vkTools::screenToWorldPos(screenPos, inverseProjection, inverseView);

        return Ray(camera->getWorldPosition(), worldSpace, distance);
     }

}
