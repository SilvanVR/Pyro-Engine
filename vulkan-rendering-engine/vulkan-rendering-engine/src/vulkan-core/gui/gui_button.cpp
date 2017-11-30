#include "gui_button.h"

#include "Input/input.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------

    GUIButton::GUIButton(TexturePtr texture, const Rectf& rect, const Color& color, Anchor anchor)
        : GUIImage(GUIElement::BUTTON, texture, rect, color, anchor)
    {}

    GUIButton::GUIButton(TexturePtr texture, const Vec2f& position, float pixelWidth, const Color& color, Anchor anchor)
        : GUIImage(GUIElement::BUTTON, texture, position, pixelWidth, color, anchor)
    {}

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Attach a lambda-function to one of the button-events which will be called then. E.g. button.attachFunc(GUIButton::BUTTONPRESSED, [&] { ... });
    void GUIButton::attachFunc(GUIButton::Event evt, const std::function<void()>& func)
    {
        switch (evt)
        {
        case Event::BUTTONPRESSED:
            buttonPressedFunctions.push_back(func); break;
        case Event::BUTTONRELEASED:
            buttonReleasedFunctions.push_back(func); break;
        case Event::BUTTONDOWN:
            buttonDownFunctions.push_back(func); break;
        case Event::MOUSEOVER:
            mouseOverFunctions.push_back(func); break;
        case Event::MOUSEIN:
            mouseInFunctions.push_back(func); break;
        case Event::MOUSEOUT:
            mouseOutFunctions.push_back(func); break;
        }
    }

    // Detach all functions from one event from this button
    void GUIButton::detachAllFuncs(GUIButton::Event evt)
    {
        switch (evt)
        {
        case Event::BUTTONPRESSED:
            buttonPressedFunctions.clear(); break;
        case Event::BUTTONRELEASED:
            buttonReleasedFunctions.clear(); break;
        case Event::BUTTONDOWN:
            buttonDownFunctions.clear(); break;
        case Event::MOUSEOVER:
            mouseOverFunctions.clear(); break;
        case Event::MOUSEIN:
            mouseInFunctions.clear(); break;
        case Event::MOUSEOUT:
            mouseOutFunctions.clear(); break;
        }
    }


    // Check if the Mouse-Button influences this button. (button-pressed, released etc.)
    void GUIButton::update(float delta)
    {
        if (screenRectangle.inside(Input::getMousePos()))
        {
            if (!m_mouseOver)
                mouseIn();

            // Left-Mouse Button pressed?
            if (Input::getKeyDownOnce(KeyCodes::LMOUSEBUTTON) || m_mouseDown)
            {
                buttonPressed();

                // Left-Mouse Button released?
                if (Input::getKeyUp(KeyCodes::LMOUSEBUTTON))
                    buttonReleased();
                return;
            }

            mouseOver();
        }
        else
        {
            if (m_mouseOver || m_mouseDown)
                mouseOut();
        }
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Called when button was pressed
    void GUIButton::buttonPressed()
    {
        // Call this functions only once when the button is pressed
        if (!m_mouseDown)
            for (auto& func : buttonPressedFunctions)
                func();

        // Call this functions every frame the button is pressed
        for (auto& func : buttonDownFunctions)
            func();

        m_mouseDown = true;
    }

    // Called when the mouse was released over the button
    void GUIButton::buttonReleased()
    {
        m_mouseDown = false;

        for (auto& func : buttonReleasedFunctions)
            func();
    }

    // Called when the mouse is over the button
    void GUIButton::mouseOver()
    {
        m_mouseOver = true;

        for (auto& func : mouseOverFunctions)
            func();
    }

    // Called when the mouse leaves the button area
    void GUIButton::mouseOut()
    {
        m_mouseOver = m_mouseDown = false;

        for (auto& func : mouseOutFunctions)
            func();
    }

    // Called when the mouse enters the button area
    void GUIButton::mouseIn()
    {
        for (auto& func : mouseInFunctions)
            func();
    }


}