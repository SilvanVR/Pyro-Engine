#ifndef GUI_COMPONENT_H_
#define GUI_COMPONENT_H_

#include "vulkan-core/data/material/material.h" 

namespace Pyro
{

    enum class Anchor
    {
        Center,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        CenterLeft,
        CenterRight,
        CenterTop,
        CenterBottom
    };

    //---------------------------------------------------------------------------
    //  GUIComponent-Class
    //---------------------------------------------------------------------------

    class GUIElement
    {
        friend class GUI; // Allow the GUI-Class to access the private "setGUI()" function.

    public:
        enum EType
        {
            TEXT,
            IMAGE,
            BUTTON,
            SLIDER
        };

        // Constructors & Destructor
        GUIElement(EType type, const Vec2f& position, const Vec2f& scale = Vec2f(1, 1), const Color& color = Color::WHITE, Anchor anchor = Anchor::TopLeft);
        GUIElement(EType type, const Rectf& rect, const Color& color, Anchor anchor);
        virtual ~GUIElement();

        // Getters
        GUIElement::EType   getType() { return type; }
        Rectf               getRect() { return screenRectangle; }
        Vec2f               getScale() { return scale; }
        Color               getColor() { return material->getColor("color"); }
        Anchor              getAnchor() { return anchor; }
        GUI*                getGUI() { return gui; }
        MaterialPtr         getMaterial() { return material; }
        bool                isActive(){ return m_isActive; }

        // Return the local x-position of the component.
        float   getX() { return this->localPosition.x(); }

        // Return the local y-position of the component.
        float   getY() { return this->localPosition.y(); }

        // Return the screen - position of the component in pixels. (Final Screen Position)
        Vec2f   getScreenPos() { return screenRectangle.getPosition(); }

        // Return the screen-rectangle, which contains the final screen position and width & height.
        Rectf&  getScreenRect() { return screenRectangle; }

        // Return the local position of the GUI-Component. (Position relative to the parents position and the used anchor)
        Vec2f   getLocalPos() { return localPosition; }

        void    setLocalPos(const Vec2f& position) { this->localPosition = position; }
        void    setX(float x) { this->localPosition.x() = x; }
        void    setY(float y) { this->localPosition.y() = y; }
        void    setScale(const Vec2f& scale) { this->scale = scale; recalculateWidthAndHeight(); }
        void    setScale(float x, float y) { scale.x() = x; scale.y() = y; recalculateWidthAndHeight(); }
        void    setColor(const Color& color) { material->setColor("color", color); }
        void    setAlpha(float alpha) { Color newCol = getColor(); newCol.a() = alpha; material->setColor("color", newCol); }
        void    setAnchor(Anchor anchor) { this->anchor = anchor; }
        void    setInheritScale(bool inheritScale) { this->inheritScale = inheritScale; }
        void    setActive(bool active){ m_isActive = active; }
        void    toggleActive(){ m_isActive = !m_isActive; }

        // GUI-hierarchy functions
        void addChild(GUIElement* guiComponent);
        void setParent(GUIElement* parent);
        void removeParent();
        void removeChild(GUIElement* guiComponent);

        // Calculates the anchored absolute (inherited) position in screen pixels relative to the attached gui. (0,0) is Top Left.
        Vec2f getAnchoredPosition(float fbW, float fbH);

        // Return the width of the component in pixels.
        float getWidth() { return screenRectangle.width(); }

        // Return the height of the component in pixels.
        float getHeight() { return screenRectangle.height(); };

        // Recalculate the component's width & height. overriden in every subclass.
        virtual void recalculateWidthAndHeight() = 0 {};

        // Update the component.
        virtual void update(float delta) {}

    protected:
        Rectf       initialRect;        // Stores the start-position + start- width & height
        Rectf       screenRectangle;    // Contains screen-position and width & height in pixels. Represents the inherited position.
        Vec2f       localPosition;      // Local-Position of this element.
        Vec2f       scale;              // Scale-Factor for this element.
        MaterialPtr material;           // The material this element is using

        // The gui this element is attached to
        GUI*        gui;

        // Set the gui of this element. Virtual cause some gui-elements can have other gui-elements, which have to be added to the gui aswell. (e.g. slider)
        virtual void setGUI(GUI* gui) { this->gui = gui; }

    private:
        GUIElement*               parent;
        std::vector<GUIElement*>  children;

        // Component-Type like Image, Text etc.
        EType type;

        // Anchors a object to a specific window position
        Anchor anchor;

        // True if this element should be rendered
        bool m_isActive = true;

        // True: Component inherits the scale factor from the parent
        bool inheritScale;
    };





}

#endif // !GUI_COMPONENT_H_
