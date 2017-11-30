#include "input_manager.h"

#include <algorithm>

namespace Pyro {

    #define floatTreshold   0.001f
    #define AXIS_MIN        -1.0f
    #define AXIS_MAX        1.0f

    void incrementAxisValue(float& val, float invAcceleration, float delta);
    void decrementAxisValue(float& val, float invAcceleration, float delta);
    void clampAxisValue(float& val, float invAcceleration, float delta);

    //---------------------------------------------------------------------------
    //  Static Members
    //---------------------------------------------------------------------------

    bool InputManager::keyDown[Input::numKeyCodes] = {};

    Vec2i InputManager::currentMousePos{ 0, 0 };

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void InputManager::update(float delta)
    {
        // Set up "Key-Released" !! ORDER DEPENDANT !!
        memset(Input::upKeys, 0, Input::numKeyCodes * sizeof(bool));
        for (int i = 0; i < Input::numKeyCodes; i++)
        {
            if (!keyDown[i] && Input::currentKeys[i])
            {
                Input::upKeys[i] = true;
                for (auto& cbInfo : Input::releasedCallbacks[i])
                    cbInfo.callback();
            }
        }

        // Set up "Key-Pressed once" !! ORDER DEPENDANT !!
        memset(Input::downKeys, 0, Input::numKeyCodes * sizeof(bool));
        for (int i = 0; i < Input::numKeyCodes; i++)
        {
            if (keyDown[i] && !Input::currentKeys[i])
            {
                Input::downKeys[i] = true;
                for (auto& cbInfo : Input::pressedOnceCallbacks[i])
                    cbInfo.callback();
            }
        }

        // Save which key was pressed this frame !! ORDER DEPENDANT !!
        memset(Input::currentKeys, 0, Input::numKeyCodes * sizeof(bool));
        for (int i = 0; i < Input::numKeyCodes; i++)
        {
            if (keyDown[i])
            {
                Input::currentKeys[i] = true;
                for (auto& cbInfo : Input::pressedCallbacks[i])
                    cbInfo.callback();
            }
        }

        // Set delta mouse position !! ORDER DEPENDANT !!
        Input::deltaMousePos = currentMousePos - Input::mousePos;

        // Set the mouse position !! ORDER DEPENDANT !!
        Input::mousePos = currentMousePos;

        // Update wheel delta in discrete timesteps
        static float updateWheelDeltaTime = 0.0f;
        static float timesUpdateWheelDelta = 1.0f / 60.0f;
        updateWheelDeltaTime += delta;
        if (updateWheelDeltaTime > timesUpdateWheelDelta)
        {
            updateWheelDeltaTime -= timesUpdateWheelDelta;

            // Check if last frame was scrolled and reset it if so !! ORDER DEPENDANT !!
            static bool scrolled = false;
            if (scrolled)
            {
                Input::wheelDelta = 0;
                scrolled = false;
            }

            // Check if mouse wheel is scrolling !! ORDER DEPENDANT !!
            if (Input::wheelDelta != 0)
                scrolled = true;

            calculateWheelAxisAmount(timesUpdateWheelDelta);
        }

        calculateAxisAmount(delta);
        calculateMouseAxisAmount(delta);
    }

    void InputManager::setKeyDown(unsigned int keyCode)
    {
        keyDown[keyCode] = true;
    }

    void InputManager::setKeyUp(unsigned int keyCode)
    {
        keyDown[keyCode] = false;
    }

    void InputManager::setMousePos(unsigned int newX, unsigned int newY)
    {
        currentMousePos.x() = newX;
        currentMousePos.y() = newY;
    }

    void InputManager::setWheelDelta(short wheelDelta)
    {
        Input::wheelDelta = wheelDelta;
    }

    void InputManager::calculateAxisAmount(float delta)
    {
        for (const auto& axis : Input::axisInformations)
        {
            float val = Input::axisMap[axis.name];

            if (Input::getKeyDown(axis.keyCodeA))
            {
                val -= axis.acceleration * delta;
                val = std::max(val, AXIS_MIN);
            }
            else
            {
                incrementAxisValue(val, axis.acceleration, delta);
            }

            if (Input::getKeyDown(axis.keyCodeB))
            {
                val += axis.acceleration * delta;
                val = std::min(val, AXIS_MAX);
            }
            else
            {
                decrementAxisValue(val, axis.acceleration, delta);
            }

            Input::axisMap[axis.name] = val;
        }
    }

    void InputManager::calculateWheelAxisAmount(float delta)
    {
        static float acceleration = 50.0f;
        static float invAcceleration = acceleration * 0.1f;

        float& val = Input::wheelAxis;

        if (Input::getWheelDelta() == 1)
        {
            val += acceleration * delta;
            val = std::min(val, AXIS_MAX);
        }
        else
        {
            decrementAxisValue(val, invAcceleration, delta);
        }

        if (Input::getWheelDelta() == -1)
        {
            val -= acceleration * delta;
            val = std::max(val, AXIS_MIN);
        }
        else
        {
            incrementAxisValue(val, invAcceleration, delta);
        }
    }

    void InputManager::calculateMouseAxisAmount(float delta)
    {
        static const float acceleration = 5.0f;
        static const float invAcceleration = 5.0f;

        const Vec2i& currentDeltaMouse = Input::deltaMousePos;

        int deltaMouseX = currentDeltaMouse.x();
        float& xVal = Input::mouseAxis.x();
        if (std::abs(deltaMouseX) > 0)
        {
            xVal += deltaMouseX * delta * acceleration;
            Mathf::clamp(xVal, AXIS_MIN, AXIS_MAX);
        }
        else
        {
            clampAxisValue(xVal, invAcceleration, delta);
        }

        int deltaMouseY = currentDeltaMouse.y();
        float& yVal = Input::mouseAxis.y();
        if (std::abs(deltaMouseY) > 0)
        {
            yVal += deltaMouseY * delta * acceleration * 0.5f;
            Mathf::clamp(yVal, AXIS_MIN, AXIS_MAX);
        }
        else
        {
            clampAxisValue(yVal, invAcceleration, delta);
        }
    }

    void incrementAxisValue(float& val, float invAcceleration, float delta)
    {
        if (val < floatTreshold)
        {
            val += invAcceleration * delta;
            if (val > floatTreshold)
                val = 0.0f;
        }
    }

    void decrementAxisValue(float& val, float invAcceleration, float delta)
    {
        if (val > floatTreshold)
        {
            val -= invAcceleration * delta;
            if (val < floatTreshold)
                val = 0.0f;
        }
    }

    void clampAxisValue(float& val, float invAcceleration, float delta)
    {
        if (val > floatTreshold)
            decrementAxisValue(val, invAcceleration, delta);
        else 
            incrementAxisValue(val, invAcceleration, delta);
    }


}