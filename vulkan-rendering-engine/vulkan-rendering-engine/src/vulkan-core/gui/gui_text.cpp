#include "gui_text.h"

#include "vulkan-core/resource_manager/resource_manager.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------

    GUIText::GUIText(const std::string& text, const Vec2f& position, FontPtr font, TextAlign align)
        : GUIText(text, position, font, Color::WHITE, Vec2f(1, 1), Anchor::TopLeft, align)
    {}

    GUIText::GUIText(const std::string& text, const Vec2f& position, const Color& color, TextAlign align)
        : GUIText(text, position, FONT_GET_DEFAULT(), color, Vec2f(1, 1), Anchor::TopLeft, align)
    {}

    GUIText::GUIText(const std::string& text, const Vec2f& position, FontPtr font, const Color& color, Anchor anchor)
        : GUIText(text, position, font, color, Vec2f(1, 1), anchor, TextAlign::LEFT)
    {}

    GUIText::GUIText(const std::string& _text, const Vec2f& position, FontPtr _font, const Color& color, const Vec2f& scale, Anchor anchor, TextAlign _align)
        : GUIElement(GUIElement::TEXT, position, scale, color, anchor), text(_text), font(_font), align(_align)
    {
        if (this->font == nullptr)
            this->font = FONT_GET_DEFAULT();

        recalculateWidthAndHeight();

        material->setTexture("tex", getFont());
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    GUIText::~GUIText()
    {}

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void GUIText::setFont(FontPtr font)
    {
        // Set new font
        if (font == nullptr)
            this->font = FONT_GET_DEFAULT();
        else
            this->font = font;

        // Font has changed. Recalculate Pixel Width & Height.
        recalculateWidthAndHeight();

        // Update Descriptor-Set with the new font-texture
        material->setTexture("tex", getFont());
    }

}