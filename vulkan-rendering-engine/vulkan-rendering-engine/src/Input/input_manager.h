/*
*  InputManager - Class header file.
*  It is the interface to the static Input-Class (where you get the information about which button is pressed and so on)
*  It implementes the singleton pattern and should only be used in os-specific code which determines what button is pressed 
*  and pass this information to this class.
*
*  Date:    22.04.2016
*  Creator: Silvan Hau
*/


#ifndef INPUT_MANAGER_H_
#define INPUT_MANAGER_H_

#include "input.h"

namespace Pyro {

    //---------------------------------------------------------------------------
    //  Input Manager Class
    //---------------------------------------------------------------------------

    class InputManager
    {
    public:
        InputManager() {};
        ~InputManager() {};

        // Update time manager once at the beginning per frame
        static void update(float delta);

        // Called from window-class
        static void setKeyDown(unsigned int keyCode);
        static void setKeyUp(unsigned int keyCode);
        static void setMousePos(unsigned int newX, unsigned int newY);
        static void setWheelDelta(short wheelDelta);
        //static void setCursor(bool setActive);

    private:
        // Used to keep track which button is currently pressed
        static bool keyDown[Input::numKeyCodes];

        // Actual mouse position which the window-class updates and used for calculating the delta to the last frame
        static Vec2i currentMousePos;

        // Calculate the axis-value for all created axises in the Input-Class
        static void calculateAxisAmount(float delta);

        // Calculate the axis-value for the wheel
        static void calculateWheelAxisAmount(float delta);

        // Calculate the mouse-axis-values
        static void calculateMouseAxisAmount(float delta);
    };

} 

#endif // !INPUT_MANAGER_H_

