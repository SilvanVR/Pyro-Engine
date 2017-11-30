#ifndef MOUSE_PICKER_H_
#define MOUSE_PICKER_H_

#include "build_options.h"

#include "vulkan-core/scene_graph/layers/layer_mask.h"
#include "math/math_interface.h"
#include "ray.h"

namespace Pyro
{

    class Camera;

    //---------------------------------------------------------------------------
    //  MousePicker Class
    //---------------------------------------------------------------------------

    class MousePicker
    {

    public:
        MousePicker();
        ~MousePicker() {}

        // Calculates and return the current mouse-ray from the main-camera
        static Ray getCurrentRay(float distance = Ray::DISTANCE_MAX);

        // Return a HitInfo which contains the hit-position or the node which were hit
        // It ignores all layers in the given layer-mask
        static HitInfo raycast(const Ray& ray = getCurrentRay(Ray::DISTANCE_MAX), LayerMask layerMask = LayerMask({ LAYER_IGNORE_RAYCASTS }));

    private:
        // Calculates the current mouse-ray in world-space from the given camera
        static Ray calculateMouseRay(Camera* camera, float distance);
    };


}



#endif // !MOUSE_PICKER_H_
