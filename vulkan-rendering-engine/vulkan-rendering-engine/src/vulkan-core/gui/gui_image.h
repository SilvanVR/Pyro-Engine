#ifndef GUI_IMAGE_H_
#define GUI_IMAGE_H_

#include "gui_element.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  GUIImage Class
    //---------------------------------------------------------------------------

    class GUIImage : public GUIElement
    {
    protected:
        // Constructors for inherited classes like the button
        GUIImage(GUIElement::EType type, TexturePtr texture, const Rectf& rect, const Color& color, Anchor anchor);
        GUIImage(GUIElement::EType type, TexturePtr texture, const Vec2f& position, float pixelWidth,
                 const Color& color = Color::WHITE, Anchor anchor = Anchor::TopLeft);

    public:
        // Constructors & Destructor
        GUIImage(TexturePtr texture, const Rectf& rect, const Color& color = Color::WHITE, Anchor anchor = Anchor::TopLeft);
        GUIImage(TexturePtr texture, const Vec2f& position, float pixelWidth, const Color& color = Color::WHITE, Anchor anchor = Anchor::TopLeft);
        virtual ~GUIImage();

        // Return the used texture
        TexturePtr getTexture() { return texture; }

        // Calculate the width and height of the image in pixels.
        void recalculateWidthAndHeight() override;

        // Set the width of the image in pixels.
        void setWidth(float pixelWidth, bool keepHeight = false);

        // Set the height of the image in pixels.
        void setHeight(float pixelHeight, bool keepWidth = false);

        // Set the texture to the given texture.
        void setTexture(TexturePtr texture);

    private:
        TexturePtr texture;     // The texture this image is using.

        void init();
    };

}



#endif // !GUI_IMAGE_H_
