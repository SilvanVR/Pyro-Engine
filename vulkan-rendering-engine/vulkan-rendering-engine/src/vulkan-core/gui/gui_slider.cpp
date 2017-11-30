#include "gui_slider.h"

#include "gui.h"
#include "Input/input.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    GUISlider::GUISlider(TexturePtr backgroundTex, float _length, TexturePtr texture, const Rectf& sliderRect, const Color& color, Orientation _orientation, Anchor anchor)
        : GUIElement(GUIElement::SLIDER, Vec2f(sliderRect.x(), sliderRect.y()), Vec2f(1, 1), color, anchor), orientation(_orientation), length(_length)
    {
        // Both will be deleted from the attached gui itself
        backgroundImage = new GUIImage(backgroundTex, Rectf(_orientation == Orientation::HORIZONTAL ? sliderRect.width() + _length : sliderRect.width(),
                                       _orientation == Orientation::HORIZONTAL ? sliderRect.height() : sliderRect.height() + _length), color, anchor);
        sliderRectImage = new GUIImage(texture, Rectf(sliderRect.width(), sliderRect.height()), color, anchor);

        // Adapt Position of the background-Image
        switch (orientation)
        {
        case Orientation::HORIZONTAL:
            backgroundImage->setLocalPos(Vec2f(-backgroundImage->getWidth() / 2 + sliderRect.width() / 2, 0));
            break;
        case Orientation::VERTICAL:
            backgroundImage->setLocalPos(Vec2f(0, -backgroundImage->getHeight() / 2 + sliderRect.height() / 2));
            break;
        }

        backgroundImage->setParent(this);
        sliderRectImage->setParent(this);

        min = 0.0f;
        max = 1.0f;

        calculateValue();
    }


    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Check if the Mouse-Button influences this button. (button-pressed, released etc.)
    void GUISlider::update(float delta)
    {
        if (sliderRectImage->getScreenRect().inside(Input::getMousePos()))
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
            if (m_mouseOver)
                mouseOut();

            if (m_mouseDown)
            {
                buttonPressed();
                if (Input::getKeyUp(KeyCodes::LMOUSEBUTTON))
                    buttonReleased();
            }
        }
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Called when button is pressed
    void GUISlider::buttonPressed()
    {
        m_mouseDown = true;

        Vec2f localPos = sliderRectImage->getLocalPos();
        Vec2f mouseDelta = (Vec2f)Input::getDeltaMousePos();

        Vec2f finalPos = localPos;

        switch (orientation)
        {
        case Orientation::HORIZONTAL:
            finalPos.x() += mouseDelta.x();
            Mathf::clamp(finalPos.x(), initialRect.x() - (length / 2) - getLocalPos().x(), initialRect.x() + (length / 2) - getLocalPos().x());
            break;
        case Orientation::VERTICAL:
            finalPos.y() += mouseDelta.y();
            Mathf::clamp(finalPos.y(), initialRect.y() - (length / 2) - getLocalPos().y(), initialRect.y() + (length / 2) - getLocalPos().y());
            break;
        }
        sliderRectImage->setLocalPos(finalPos);

        calculateValue();
    }

    // Called when the mouse was released
    void GUISlider::buttonReleased()
    {
        m_mouseDown = false;
    }

    // Called when the mouse is over the button
    void GUISlider::mouseOver()
    {
        m_mouseOver = true;
    }

    // Called when the mouse leaves the button area
    void GUISlider::mouseOut()
    {
        m_mouseOver = false;
    }

    // Called when the mouse enters the button area
    void GUISlider::mouseIn()
    {
    }

    // Calculate the slider-value based on the position of the slider, the min- & max- value and call the functions bound to this slider
    void GUISlider::calculateValue()
    {
        Vec2f localPos = sliderRectImage->getLocalPos();
        switch (orientation)
        {
        case Orientation::HORIZONTAL:
            value = (localPos.x() + length / 2) / length;
            break;
        case Orientation::VERTICAL:
            value = (localPos.y() + length / 2) / length;
            break;
        }

        value = Mathf::lerp(max, min, value);

        for (auto& func : sliderMovedFunctions)
            func(value);
    }


    // Add both images to the GUI.
    void GUISlider::setGUI(GUI* gui)
    {
        this->gui = gui;
        gui->add(sliderRectImage);       // Add the slider-image to the GUI
        gui->add(backgroundImage, true); // Add background image to the GUI and render it BEFORE the slider-rect
    }

}
