#include "gui_element.h"

#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "gui.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    GUIElement::GUIElement(EType _type, const Vec2f& _position, const Vec2f& _scale, const Color& color, Anchor _anchor)
        : type(_type), localPosition(_position), scale(_scale), anchor(_anchor), inheritScale(true), initialRect(_position.x(), _position.y(), 0, 0)
    {
        material = MATERIAL(SHADER("GUI"), "GUI-Material");
        setColor(color);
    }

    GUIElement::GUIElement(EType _type, const Rectf& rect, const Color& color, Anchor _anchor)
        : type(_type), localPosition(rect.x(), rect.y()), screenRectangle(rect), scale(Vec2f(1, 1)), anchor(_anchor), inheritScale(true), initialRect(rect)
    {
        material = MATERIAL(SHADER("GUI"), "GUI-Material");
        setColor(color);
    }

    GUIElement::~GUIElement()
    { 
        //removeParent();
        if(gui != nullptr)
            gui->remove(this);
    }

    //---------------------------------------------------------------------------
    //  Public-Methods
    //---------------------------------------------------------------------------

    // Add a child to the gui-component, which inherits the position of the parent.
    void GUIElement::addChild(GUIElement* guiElement)
    {
        children.push_back(guiElement);
        guiElement->setParent(this);
    }

    // Set the parent of this gui-component. This component inherits the position of the parent.
    void GUIElement::setParent(GUIElement* guiElement)
    {
        this->parent = guiElement;
    }

    // Remove the parent of this component.
    void GUIElement::removeParent()
    {
        if (parent != nullptr)
            parent->removeChild(this);
        parent = nullptr;
    }

    // Remove the given child from this component.
    void GUIElement::removeChild(GUIElement* guiElement)
    {
        guiElement->setParent(nullptr);
        removeObjectFromList(children, guiElement);
    }

    // Calculates the anchored absolute (inherited) position in screen pixels and returns it. (0,0) is Top Left.
    Vec2f GUIElement::getAnchoredPosition(float fbW, float fbH)
    {
        // Reset current screen-position and recalculate it
        screenRectangle.x() = 0;
        screenRectangle.y() = 0;

        // Get Anchored-Position from the Parent and use parents-width & height to calculate the local anchored position.
        if (parent != nullptr)
        {
            screenRectangle.setPosition(parent->getAnchoredPosition(fbW, fbH));
            fbW = parent->getWidth();
            fbH = parent->getHeight();
            if (inheritScale)
                setScale(parent->getScale());
        }
        else // Root - Node reached. Take Position from the GUI.
        {
            screenRectangle.setPosition(gui->getPos());
        }

        // Calculate anchored position
        switch (anchor)
        {
        case Anchor::Center:
            screenRectangle.x() += localPosition.x() + fbW / 2.0f;
            screenRectangle.y() += localPosition.y() + fbH / 2.0f;
            break;
        case Anchor::TopLeft:
            screenRectangle.x() += localPosition.x();
            screenRectangle.y() += localPosition.y();
            break;
        case Anchor::BottomLeft:
            screenRectangle.x() += localPosition.x();
            screenRectangle.y() += localPosition.y() + fbH;
            break;
        case Anchor::BottomRight:
            screenRectangle.x() += localPosition.x() + fbW;
            screenRectangle.y() += localPosition.y() + fbH;
            break;
        case Anchor::TopRight:
            screenRectangle.x() += localPosition.x() + fbW;
            screenRectangle.y() += localPosition.y();
            break;
        case Anchor::CenterLeft:
            screenRectangle.x() += localPosition.x();
            screenRectangle.y() += localPosition.y() + fbH / 2.0f;
            break;
        case Anchor::CenterRight:
            screenRectangle.x() += localPosition.x() + fbW;
            screenRectangle.y() += localPosition.y() + fbH / 2.0f;
            break;
        case Anchor::CenterTop:
            screenRectangle.x() += localPosition.x() + fbW / 2.0f;
            screenRectangle.y() += localPosition.y();
            break;
        case Anchor::CenterBottom:
            screenRectangle.x() += localPosition.x() + fbW / 2.0f;
            screenRectangle.y() += localPosition.y() + fbH;
            break;
        }

        return screenRectangle.getPosition();
    }


}