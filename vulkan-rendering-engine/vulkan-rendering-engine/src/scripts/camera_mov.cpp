#include "camera_mov.h"

#include "Input/input.h"

namespace Pyro
{

#define SMOOTH_ROTATION 0

    CMoveCamera::CMoveCamera(float _moveAmt, float _mouseSensitivity, float acceleration, ECameraMode _mode)
        : moveAmt(_moveAmt), mouseSensitivity(-_mouseSensitivity*0.1f), mode(_mode)
    {
        Input::addAxis("cam_horizontal", KeyCodes::A, KeyCodes::D, acceleration);
        Input::addAxis("cam_vertical", KeyCodes::S, KeyCodes::W, acceleration);
        Input::addAxis("cam_up", KeyCodes::LEFT_SHIFT, KeyCodes::SPACE, acceleration);

        Input::addAxis("cam_rot_horizontal", KeyCodes::LEFT, KeyCodes::RIGHT, acceleration);
        Input::addAxis("cam_rot_vertical", KeyCodes::DOWN, KeyCodes::UP, acceleration);
        fastMoveAmt = 5 * moveAmt;
    }

    void CMoveCamera::addedToNode(Node* node)
    {
        //trans = &parentNode->getTransform();
    }

    void CMoveCamera::update(float delta)
    {
        Transform* trans = &parentNode->getTransform();

        if(Input::getKeyDownOnce(KeyCodes::ONE))
            mode = ECameraMode::FPS;
        else if (Input::getKeyDownOnce(KeyCodes::TWO))
        {
            pointOfInterest = trans->position + trans->rotation.getForward() * 50.0f;
            mode = ECameraMode::MAYA;
        }

        float amt = moveAmt;
        if (Input::getKeyDown(KeyCodes::LEFT_CONTROL))
            amt = fastMoveAmt;

        float arrowKeyRotScale = 200;
        switch (mode)
        {
        case ECameraMode::FPS:
            // Smooth-Movement
            trans->position += Input::getAxis("cam_horizontal") * delta * amt * trans->rotation.getRight();
            trans->position += Input::getAxis("cam_vertical") * delta * amt * trans->rotation.getForward();
            trans->position += Input::getAxis("cam_up")  * delta * amt * trans->rotation.getUp();

            // Smoth-Rotation with arrow-keys
            trans->rotation *= Quatf(Vec3f::down, Mathf::deg2Rad(Input::getAxis("cam_rot_horizontal") * arrowKeyRotScale * delta));
            trans->rotation *= Quatf(trans->rotation.getRight(), Mathf::deg2Rad(Input::getAxis("cam_rot_vertical") * arrowKeyRotScale * delta));

#if SMOOTH_ROTATION
            // Smooth rotation using the mouse-axis
            if (Input::getKeyDown(KeyCodes::RMOUSEBUTTON))
            {  
                const Vec2f& mouseAxis = Input::getMouseAxis();
                const float fixVal = 500.0f;
                trans->rotation *= Quatf(trans->rotation.getRight(), Mathf::deg2Rad(mouseAxis.y() * mouseSensitivity * fixVal * delta));
                trans->rotation *= Quatf::rotationY(Mathf::deg2Rad(mouseAxis.x() * mouseSensitivity * fixVal * delta));
            }
#else
            // Rotation with mouse using delta-mouse
            if (Input::getKeyDown(KeyCodes::RMOUSEBUTTON))
            {
                const Vec2i& deltaMouse = Input::getDeltaMousePos();

                trans->rotation *= Quatf(trans->rotation.getRight(), Mathf::deg2Rad(deltaMouse.y() * mouseSensitivity));
                trans->rotation *= Quatf::rotationY(Mathf::deg2Rad(deltaMouse.x() * mouseSensitivity));
            }
#endif

            // Wheel-Movement
            trans->position += Input::getWheelAxis() * delta * 200.0f * trans->rotation.getForward();

            break;
        case ECameraMode::MAYA:

            float distanceToPOI = (trans->position - pointOfInterest).magnitude();

#if SMOOTH_ROTATION
            // Smooth-Camera rotation using the Mouse-Axis
            static Vec2f mouseAxisWhenUp;
            static bool up = false;
            if (Input::getKeyDown(KeyCodes::RMOUSEBUTTON) || up)
            {
                if(Input::getKeyDown(KeyCodes::RMOUSEBUTTON))
                    up = false;

                Vec2f mouseAxis = Input::getMouseAxis();
                if(up)
                    mouseAxis = mouseAxisWhenUp;

                float amt = distanceToPOI * 3.0f;
                trans->position += mouseAxis.x() * delta * amt * trans->rotation.getLeft();
                trans->position += mouseAxis.y() * delta * amt * trans->rotation.getUp();

                // Adjust rotation
                trans->lookAt(pointOfInterest);

                // Adjust distance
                trans->position = pointOfInterest - (trans->rotation.getForward() * distanceToPOI);

                if (up)
                {
                    mouseAxisWhenUp.scale(Vec2f(0.95f, 0.95f));
                    if(mouseAxis.magnitude() < 0.05f)
                        up = false;
                }
            }

            if(Input::getKeyUp(KeyCodes::RMOUSEBUTTON))
            {
                mouseAxisWhenUp = Input::getMouseAxis();
                up = true;
            }
#else
            // standard camera-rotation using delta-mouse
            if (Input::getKeyDown(KeyCodes::RMOUSEBUTTON))
            {
                const Vec2i& deltaMouse = Input::getDeltaMousePos();

                // The further away the faster the rotation
                float amt = distanceToPOI * 0.005f;
                trans->position += trans->rotation.getUp() * (float)deltaMouse.y() * amt;
                trans->position += trans->rotation.getLeft() * (float)deltaMouse.x() * amt;

                // Adjust rotation
                trans->lookAt(pointOfInterest);

                // Adjust distance
                trans->position = pointOfInterest - (trans->rotation.getForward() * distanceToPOI);
            }
#endif


            // Zoom-In & -Out with mouse-wheel
            float wheelAmt = distanceToPOI * 4.0f;
            trans->position += Input::getWheelAxis() * delta * wheelAmt * trans->rotation.getForward();

            break;
        }



        if (Input::getKeyDown(KeyCodes::R))
        {
            pointOfInterest = Point3f();
            trans->lookAt(pointOfInterest);
        }

    }

}


// if (Input::getWheelDelta())
//     trans->position += Input::getWheelDelta() * delta * trans->rotation.getForward() * 2000;
// Old-Code. May be removed sometime
//if (Input::getKeyDown(KeyCodes::W))
//{
//    trans->position += moveAmt * delta * trans->rotation.getForward();
//}
//if (Input::getKeyDown(KeyCodes::S))
//{
//    trans->position += moveAmt * delta * trans->rotation.getBack();
//}
//if (Input::getKeyDown(KeyCodes::A))
//{
//    trans->position += moveAmt * delta * trans->rotation.getLeft();
//}
//if (Input::getKeyDown(KeyCodes::D))
//{
//    trans->position += moveAmt * delta * trans->rotation.getRight();
//}
//if (Input::getKeyDown(KeyCodes::SPACE))
//{
//    trans->position += moveAmt * delta * trans->rotation.getUp();
//}
//if (Input::getKeyDown(KeyCodes::LEFT_SHIFT))
//{
//    trans->position += moveAmt * delta * trans->rotation.getDown();
//}