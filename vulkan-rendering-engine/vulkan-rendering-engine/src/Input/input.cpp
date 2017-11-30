#include "input.h"

#include <assert.h>
#include "logger/logger.h"
#include "utils/utils.h"

namespace Pyro {

    // ID-Generator for function-objects attached to the Input-System
    static IDGenerator<CallbackID> idGenerator;

    //---------------------------------------------------------------------------
    //  Static Members
    //---------------------------------------------------------------------------

    bool    Input::currentKeys[Input::numKeyCodes] = {};
    bool    Input::downKeys[Input::numKeyCodes] = {};
    bool    Input::upKeys[Input::numKeyCodes] = {};

    Vec2i   Input::deltaMousePos{ 0,0 };
    Vec2i   Input::mousePos{ 0,0 };
    Vec2f   Input::mouseAxis{ 0.0f, 0.0f };

    short   Input::wheelDelta = 0;
    float   Input::wheelAxis = 0.0f;

    std::vector<CallbackInfo> Input::pressedCallbacks[Input::numKeyCodes];
    std::vector<CallbackInfo> Input::pressedOnceCallbacks[Input::numKeyCodes];
    std::vector<CallbackInfo> Input::releasedCallbacks[Input::numKeyCodes];

    std::vector<AxisInfo> Input::axisInformations;
    std::map<std::string, float> Input::axisMap;

    //---------------------------------------------------------------------------
    //  Static Methods
    //---------------------------------------------------------------------------

    // Return always true if the given Key is pressed down.
    bool Input::getKeyDown(int keyCode)
    {
        return currentKeys[keyCode];
    }

    // Return true in the frame the button is pressed 
    bool Input::getKeyDownOnce(int keyCode)
    {
        return downKeys[keyCode];
    }

    // Return true in the frame the button is released 
    bool Input::getKeyUp(int keyCode)
    {
        return upKeys[keyCode];
    }

    // Return the actual mouse position as a Vector2. (0,0) top left corner.
    const Vec2i& Input::getMousePos()
    {
        return mousePos;
    }

    // Return the difference of the mouse position from the last frame
    const Vec2i& Input::getDeltaMousePos()
    {
        return deltaMousePos;
    }

    // Return the wheel delta
    short Input::getWheelDelta()
    {
        return wheelDelta;
    }

    // Return the amount of the given axis
    float Input::getAxis(const std::string& name)
    {
        assert(axisMap.count(name) > 0 && "Given Axis-Name does not exist.");
        return axisMap[name];
    }

    // Return the amount of the wheel axis
    float Input::getWheelAxis()
    {
        return wheelAxis;
    }

    // Return the amount of the mouse-axis in both directions (x and y)
    const Vec2f& Input::getMouseAxis()
    {
        return mouseAxis;
    }

    // Attach a lambda-function to the input-system. ECallback::KEY_DOWN called every frame the key is down.
    // ECallback::KEY_PRESSED called only once when key is pressed.
    CallbackID Input::attachFunc(int keyCode, const std::function<void()>& func, ECallbackEvent _event)
    {
        CallbackInfo info;
        info.callback = func;
        info.id = idGenerator.generateID();

        switch (_event)
        {
        case ECallbackEvent::KEY_PRESSED:
            pressedOnceCallbacks[keyCode].push_back(info); break;
        case ECallbackEvent::KEY_DOWN:
            pressedCallbacks[keyCode].push_back(info); break;
        case ECallbackEvent::KEY_UP:
            releasedCallbacks[keyCode].push_back(info); break;
        }

        return info.id;
    }

    void Input::detachFunc(CallbackID id)
    {
        idGenerator.freeID(id);

        // Loop through all callbacks, find that id and delete it
        for (uint32_t i = 0; i < numKeyCodes; i++)
        {
            pressedOnceCallbacks[i].erase(std::remove_if(pressedOnceCallbacks[i].begin(), pressedOnceCallbacks[i].end(),
                                          [=](const CallbackInfo& info) { return id == info.id; }), pressedOnceCallbacks[i].end());

            pressedCallbacks[i].erase(std::remove_if(pressedCallbacks[i].begin(), pressedCallbacks[i].end(),
                                      [=](const CallbackInfo& info) { return id == info.id; }), pressedCallbacks[i].end());

            releasedCallbacks[i].erase(std::remove_if(releasedCallbacks[i].begin(), releasedCallbacks[i].end(),
                                       [=](const CallbackInfo& info) { return id == info.id; }), releasedCallbacks[i].end());
        }
    }

    void Input::addAxis(const std::string& name, int keyCodeA, int keyCodeB, float acc)
    {
        //assert(Input::axisMap.count(name) == 0); due to scene-changing scripts can add axis's more than once
        if (Input::axisMap.count(name) == 0)
        {
            axisInformations.push_back({ name, keyCodeA, keyCodeB, acc });
            Input::axisMap[name] = 0.0f;
        } else {
            Logger::Log("Axis with name '" + name + "' was already registered!", LOGTYPE_WARNING, LOG_LEVEL_NOT_IMPORTANT);
        }
    }

}