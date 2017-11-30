#include "gui_image.h"

#include <assert.h>
#include "vulkan-core/data/material/texture/texture.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    GUIImage::GUIImage(TexturePtr texture, const Vec2f& position, float pixelWidth, const Color& color, Anchor anchor)
        : GUIImage(GUIElement::IMAGE, texture, position, pixelWidth, color, anchor)
    {}

    GUIImage::GUIImage(TexturePtr _texture, const Rectf& rect, const Color& color, Anchor anchor)
        : GUIElement(GUIElement::IMAGE, rect, color, anchor), texture(_texture)
    {
        init();
        bool calculateWidth = rect.width() == 0;
        bool calculateHeight = rect.height() == 0;

        if (calculateHeight)
            setWidth(rect.width());
        else if (calculateWidth)
            setHeight(rect.height());
        else if (calculateHeight && calculateWidth)
            assert(0 && "RECT WIDTH & HEIGHT in GUIImage-Constructor are BOTH 0!!");
    }

    //---------------------------------------------------------------------------
    //  Protected Constructor
    //---------------------------------------------------------------------------

    GUIImage::GUIImage(GUIElement::EType type, TexturePtr _texture, const Rectf& rect, const Color& color, Anchor anchor)
        : GUIElement(type, rect, color, anchor), texture(_texture)
    {
        init();
    }

    GUIImage::GUIImage(GUIElement::EType type, TexturePtr _texture, const Vec2f& position, float pixelWidth, const Color& color, Anchor anchor)
        : GUIElement(type, position, Vec2f(1, 1), color, anchor), texture(_texture)
    {
        init();
        setWidth(pixelWidth);

        initialRect.x() = position.x();
        initialRect.y() = position.y();
        initialRect.width() = pixelWidth;
        initialRect.height() = screenRectangle.height();
    }

    void GUIImage::init()
    {
        // Image sampler + Uniform
        material->setTexture("tex", texture);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    GUIImage::~GUIImage()
    {}

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void GUIImage::setTexture(TexturePtr newTexture)
    {
        // Set new texture
        this->texture = newTexture;

        // Update Descriptor set
        material->setTexture("tex", texture);
    }

    // Calculate the width and height of the image in pixels.
    void GUIImage::recalculateWidthAndHeight()
    {
        screenRectangle.width() = initialRect.width() * scale.x();
        screenRectangle.height() = initialRect.height() * scale.y();
    }

    // Set the width of the image in pixels.
    void GUIImage::setWidth(float pixelWidth, bool keepHeight)
    {
        screenRectangle.width() = pixelWidth;
        if (!keepHeight)
            screenRectangle.height() = texture->getHeight() * pixelWidth / texture->getWidth();
    }

    // Set the height of the image in pixels.
    void GUIImage::setHeight(float pixelHeight, bool keepWidth)
    {
        screenRectangle.height() = pixelHeight;
        if (!keepWidth)
            screenRectangle.width() = texture->getWidth() * pixelHeight / texture->getHeight();
    }

}

