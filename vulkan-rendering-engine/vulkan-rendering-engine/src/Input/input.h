/*
*  Input - Class header file.
*  Basic Input class which gives you information about what button is pressed or where the mouse position is.
*  The InputManager set's the fields for this class.
*
*  Date:    22.04.2016
*  Creator: Silvan Hau
*/

#ifndef INPUT_H_
#define INPUT_H_

#include "math/math_interface.h"
#include "structs.hpp"
#include <map>

namespace Pyro
{

    struct AxisInfo
    {
        std::string name;
        int keyCodeA;
        int keyCodeB;
        float acceleration;
    };

    //---------------------------------------------------------------------------
    //  InputManager class
    //---------------------------------------------------------------------------

    class Input
    {
        friend class InputManager;  // Allow the InputManager to access the data fields directly

    public:
        enum ECallbackEvent
        {
            KEY_PRESSED,
            KEY_DOWN,
            KEY_UP,
        };

        static bool getKeyDown(int keyCode);                // Return always true if the given Key is pressed down
        static bool getKeyDownOnce(int keyCode);            // Return true in the frame the button is pressed 
        static bool getKeyUp(int keyCode);                  // Return true in the frame the button is released 
        static const Vec2i& getDeltaMousePos();             // Return the difference of the mouse position from the last frame.
        static const Vec2i& getMousePos();                  // Return the actual mouse position as a Vector2. (0,0) top left corner
        static short getWheelDelta();                       // Return the actual wheel delta.
        static float getAxis(const std::string& name);      // Return the amount of the given axis
        static float getWheelAxis();                        // Return the amount of the wheel axis
        static const Vec2f& getMouseAxis();                 // Return the amount of the mouse-axis in both directions (x and y)

        // Attach a lambda-function to the input-system. ECallback::KEY_DOWN called every frame the key is down.
        // ECallback::KEY_PRESSED called only once when key is pressed.
        static CallbackID attachFunc(int keyCode, const std::function<void()>& func, ECallbackEvent _event = ECallbackEvent::KEY_PRESSED);

        // Detach a functor from the Input-System. The "id" is the returned id by "attachFunc()"
        static void detachFunc(CallbackID id);

        // Add an axis to the input-system. The value of the axis interpolated between "min" and "max" whether
        // one of the given Keys is pressed. "keyCodeA" corresponds to "min" and "keyCodeB" to "max".
        // Acceleration gives the amount of how fast the axis-value changes.
        static void addAxis(const std::string& name, int keyCodeA, int keyCodeB, float acc);

    private:
        static const int numKeyCodes = 1024;

        static bool currentKeys[numKeyCodes];
        static bool downKeys[numKeyCodes];
        static bool upKeys[numKeyCodes];

        // Stores lambda-functions attached to an button-event, which will be called from the Input-Manager
        static std::vector<CallbackInfo> pressedCallbacks[numKeyCodes];
        static std::vector<CallbackInfo> pressedOnceCallbacks[numKeyCodes];
        static std::vector<CallbackInfo> releasedCallbacks[numKeyCodes];

        static Vec2i deltaMousePos;
        static Vec2i mousePos;
        static Vec2f mouseAxis;

        static short wheelDelta;
        static float wheelAxis;

        // Stores the information about one axis event
        static std::vector<AxisInfo> axisInformations;

        // Stores the axis with the value
        static std::map<std::string, float> axisMap;
    };



    //---------------------------------------------------------------------------
    //  KeyCode Enum
    //---------------------------------------------------------------------------

    enum KeyCodes
    {
        LMOUSEBUTTON = 0x0,
        RMOUSEBUTTON = 0x1,
        MMOUSEBUTTON = 0x2,
        MOUSEBUTTON3 = 0x3,
        MOUSEBUTTON4 = 0x4,
        MOUSEBUTTON5 = 0x5,
        MOUSEBUTTON6 = 0x6,
        MOUSEBUTTON7 = 0x7,
        SPACE        = 32,
        APOSTROPHE   = 39,
        COMMA        = 44,
        MINUS        = 45,
        PERIOD       = 46,
        SLASH        = 47,
        ZERO         = 48,
        ONE          = 49,
        TWO          = 50,
        THREE        = 51,
        FOUR         = 52,
        FIVE         = 53,
        SIX          = 54,
        SEVEN        = 55,
        EIGHT        = 56,
        NINE         = 57,
        SEMICOLON    = 59,
        EQUAL        = 61,
        A            = 65,
        B            = 66,
        C            = 67,
        D            = 68,
        E            = 69,
        F            = 70,
        G            = 71,
        H            = 72,
        I            = 73,
        J            = 74,
        K            = 75,
        L            = 76,
        M            = 77,
        N            = 78,
        O            = 79,
        P            = 80,
        Q            = 81,
        R            = 82,
        S            = 83,
        T            = 84,
        U            = 85,
        V            = 86,
        W            = 87,
        X            = 88,
        Y            = 90,
        Z            = 89,
        LEFT_BRACKET = 91,
        BACKSLASH    = 92,
        RIGHT_BRACKET= 93,
        GRAVE_ACCENT = 96,
        WORLD_1      = 161,
        WORLD_2      = 162,
        ESCAPE       = 256,
        ENTER        = 257,
        TAB          = 258,
        BACKSPACE    = 259,
        INSERT       = 260,
        DEL          = 261,
        RIGHT        = 262,
        LEFT         = 263,
        DOWN         = 264,
        UP           = 265,
        PAGE_UP      = 266,
        PAGE_DOWN    = 267,
        HOME         = 268,
        END          = 269,
        CAPS_LOCK    = 280,
        SCROLL_LOCK  = 281,
        NUM_LOCK     = 282,
        PRINT_SCREEN = 283,
        PAUSE        = 284,
        F1           = 290,
        F2           = 291,
        F3           = 292,
        F4           = 293,
        F5           = 294,
        F6           = 295,
        F7           = 296,
        F8           = 297,
        F9           = 298,
        F10          = 299,
        F11          = 300,
        F12          = 301,
        F13          = 302,
        F14          = 303,
        F15          = 304,
        F16          = 305,
        F17          = 306,
        F18          = 307,
        F19          = 308,
        F20          = 309,
        F21          = 310,
        F22          = 311,
        F23          = 312,
        F24          = 313,
        F25          = 314,
        KP_0         = 320,
        KP_1         = 321,
        KP_2         = 322,
        KP_3         = 323,
        KP_4         = 324,
        KP_5         = 325,
        KP_6         = 326,
        KP_7         = 327,
        KP_8         = 328,
        KP_9         = 329,
        KP_DECIMAL   = 330,
        KP_DIVIDE    = 331,
        KP_MULTIPLY  = 332,
        KP_SUBTRACT  = 333,
        KP_ADD       = 334,
        KP_ENTER     = 335,
        KP_EQUAL     = 336,
        LEFT_SHIFT   = 340,
        LEFT_CONTROL = 341,
        LEFT_ALT     = 342,
        LEFT_SUPER   = 343,
        RIGHT_SHIFT  = 344,
        RIGHT_CONTROL= 345,
        RIGHT_ALT    = 346,
        RIGHT_SUPER  = 347,
        MENU         = 348,

    };


}

#endif // !INPUT_H_