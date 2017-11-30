#ifndef TEXT_H_
#define TEXT_H_

#include "gui_element.h"
#include "font.h"

namespace Pyro
{

    enum class TextAlign { LEFT, CENTER, RIGHT };

    //---------------------------------------------------------------------------
    //  Text class
    //---------------------------------------------------------------------------

    class GUIText : public GUIElement
    {
        std::string text;       // The text to render
        TextAlign   align;      // How this text should be aligned
        FontPtr     font;       // Used font

    public:
        // Constructors
        GUIText(const std::string& _text, const Vec2f& position, FontPtr font = nullptr, TextAlign _align = TextAlign::LEFT);
        GUIText(const std::string& _text, const Vec2f& position, const Color& color, TextAlign _align = TextAlign::LEFT);
        GUIText(const std::string& _text, const Vec2f& position, FontPtr font, const Color& color, Anchor anchor);
        GUIText(const std::string& _text, const Vec2f& position, FontPtr font, const Color& color,
                const Vec2f& scale = Vec2f(1, 1), Anchor anchor = Anchor::TopLeft, TextAlign _align = TextAlign::LEFT);
        // Destructor
        ~GUIText();

        // Getter's
        std::string&    getText() { return text; }
        TextAlign       getAlign() const { return align; }
        FontPtr         getFont() const { return font; }
        std::size_t     getTextSize() const { return text.size(); }    // Return the number of letters in this text

        // Setter's
        void            setText(const std::string& text) { this->text = text; recalculateWidthAndHeight(); }
        void            setAlign(TextAlign align) { this->align = align; }
        void            setFont(FontPtr font);

        // Calculate the width and height of the text in pixels.
        void recalculateWidthAndHeight() override
        {
            screenRectangle.width() = font->getTextWidth(text) * scale.x();
            screenRectangle.height() = font->getTextHeight(text) * scale.y();
        }
    };

}

#endif // !TEXT_H_