#ifndef FPS_CAMERA_H_
#define FPS_CAMERA_H_

#include "vulkan-core/script_interface.hpp"
#include "interaction_script.h"

namespace Pyro
{

    enum class ECameraMode
    {
        FPS,
        MAYA
    };

    class CMoveCamera : public Component
    {

    public:
        CMoveCamera(float moveAmt, float mouseSensitivity, float acceleration = 5.0f, ECameraMode mode = ECameraMode::FPS);

        void update(float delta) override;

        void addedToNode(Node* node) override;

    private:
        float fastMoveAmt;
        float moveAmt;
        float mouseSensitivity;

        ECameraMode mode;

        // For Maya-Mode
        Point3f pointOfInterest;
    };

}


#endif // !FPS_CAMERA_H_
