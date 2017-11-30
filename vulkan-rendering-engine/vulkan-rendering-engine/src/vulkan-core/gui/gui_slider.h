#ifndef GUI_SLIDER_H_
#define GUI_SLIDER_H_

#include "gui_image.h"
#include "gui_element.h"

#include <functional>

namespace Pyro
{


    class GUI;

    //---------------------------------------------------------------------------
    //  GUIButton Class
    //---------------------------------------------------------------------------

    class GUISlider : public GUIElement
    {
    public:
        enum Orientation
        {
            HORIZONTAL,
            VERTICAL
        };

        // Length: Length of the slider in pixels. sliderRect: Width & Height of the small slider rect.
        GUISlider(TexturePtr backgroundTex, float length, TexturePtr sliderTex, const Rectf& sliderRect = Rectf(200, 100),
            const Color& color = Color::WHITE, GUISlider::Orientation orientation = Orientation::HORIZONTAL, Anchor anchor = Anchor::TopLeft);
        ~GUISlider() {}

        // Return the value this slider is interpolating (default: 0 - 1, can be changed with setMinMax() - function.)
        float getValue() { return value; }

        // Set the min & max - value this slider is interpolating between.
        void setMinMax(float min, float max) { this->min = min; this->max = max; }

        // Attach a lambda-function to this slider, which holds the value as a parameter and will be called when the slider-val changes 
        void attachFunc(const std::function<void(float)>& func) { sliderMovedFunctions.push_back(func); }

        // Remove all functions bound to this slider
        void detachAllFuncs() { sliderMovedFunctions.clear(); }

        // Return the background-image from this slider. Used by the GUI-Renderer
        GUIImage* getBackgroundImage() { return backgroundImage; }

        // Return the Slider-Rectangle Image.
        GUIImage* getImage() { return sliderRectImage; };

        // Check if the Mouse-Button influences this slider.
        void update(float delta) override;

        // Add both images to the GUI.
        void setGUI(GUI* gui) override;

        // Calculate the width and height of the image in pixels.
        void recalculateWidthAndHeight() override { /* Do Nothing. This Element has no width & height. */ }

    private:
        GUIImage*   sliderRectImage;    // The Slider-Rectangle Image
        GUIImage*   backgroundImage;    // The Background-Image from this slider
        Orientation orientation;        // Orientation of this Slider (Horizontal / Vertical)
        bool        m_mouseOver;        // True: Mouse is over the slider
        bool        m_mouseDown;        // True: Mouse is pressed over the slider
        float       length;             // The Length of this slider in pixels
        float       value;              // The value this slider represents. Default: 0 - 1. Can be set with setMinMax().
        float       min, max;           // The min-max - values this slider is interpolating between.

        // Accept a float value as a parameter, which is the slider-value.
        std::vector<std::function<void(float)>> sliderMovedFunctions;

        // Event-Functions
        void buttonPressed();
        void buttonReleased();
        void mouseOver();
        void mouseOut();
        void mouseIn();

        // Calculate the slider-value based on the position of the slider and the min- & max- value
        void calculateValue();

    };

}

#endif // GUI_SLIDER_H_