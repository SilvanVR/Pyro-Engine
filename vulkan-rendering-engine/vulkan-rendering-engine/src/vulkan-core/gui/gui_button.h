#ifndef GUI_BUTTON_H_
#define GUI_BUTTON_H_

#include "gui_image.h"

#include <functional>

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  GUIButton Class
    //---------------------------------------------------------------------------

    class GUIButton : public GUIImage
    {
    public:
        // The Events, where lambda-functions can attached to
        enum Event
        {
            BUTTONPRESSED,
            BUTTONRELEASED,
            BUTTONDOWN,
            MOUSEOVER,
            MOUSEIN,
            MOUSEOUT
        };

        // Constructors & Destructor
        GUIButton(TexturePtr texture, const Rectf& rect = Rectf(200, 100), const Color& color = Color::WHITE, Anchor anchor = Anchor::TopLeft);
        GUIButton(TexturePtr texture, const Vec2f& position, float pixelWidth, const Color& color = Color::WHITE, Anchor anchor = Anchor::TopLeft);
        ~GUIButton() {};

        // Return whether the mouse is over this button.
        bool isMouseOver() { return m_mouseOver; }

        // Return whether this button is pressed or not.
        bool isPressed() { return m_mouseDown; }

        // Attach a lambda-function to one of the button-events which will be called then. E.g. button.attachFunc(GUIButton::BUTTONPRESSED, [&] { ... });
        void attachFunc(GUIButton::Event evt, const std::function<void()>& func);

        // Detach all functions from one event from this button
        void detachAllFuncs(GUIButton::Event evt);

        // Check if the Mouse-Button influences this button. (button-pressed, released etc.)
        void update(float delta) override;

    private:
        bool m_mouseOver;
        bool m_mouseDown;

        std::vector<std::function<void()>> buttonPressedFunctions;
        std::vector<std::function<void()>> buttonReleasedFunctions;
        std::vector<std::function<void()>> buttonDownFunctions;
        std::vector<std::function<void()>> mouseOverFunctions;
        std::vector<std::function<void()>> mouseInFunctions;
        std::vector<std::function<void()>> mouseOutFunctions;

        void buttonPressed();
        void buttonReleased();
        void mouseOver();
        void mouseOut();
        void mouseIn();
    };


}



#endif // !GUI_BUTTON_H_