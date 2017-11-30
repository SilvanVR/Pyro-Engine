#ifndef GUI_H_
#define GUI_H_

#include "vulkan-core/scene_graph/nodes/components/component.h"

#include "gui_element.h"
#include "gui_image.h"
#include "gui_text.h"
#include "gui_button.h"
#include "gui_slider.h"

#include <vector>

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  GUI - Class
    //---------------------------------------------------------------------------

    class GUI : public Component
    {
        friend class GUIRenderer; // Allow the GUI-Renderer to access the private fields and render this GUI.

    public:
        GUI(bool isActive = true);
        GUI(GUIElement* guiElem);
        GUI(const std::vector<GUIElement*>& guiElems);
        ~GUI();

        // Add a GUI-Component to this GUI.
        void add(GUIElement* guiElem, bool renderFirst = false);

        // Add several GUI-Components to this GUI. Use with gui.add( { &comp1, &comp2, ... } );
        void add(const std::vector<GUIElement*>& guiElems);

        // Remove a GUI-Component from this GUI.
        void remove(GUIElement* guiElem);

        // Remove several GUI-Components from this GUI.
        void remove(const std::vector<GUIElement*>& guiElems);

        // Set the color for all gui-elements in this GUI.
        void setColor(const Color& color);

        // Set the alpha value for all gui-components (the whole GUI).
        void setAlpha(float alpha);

        // Set this GUI on top of all GUI's
        void onTop();

        // Set the position of this GUI, which manipulates the positions of all GUI-Elements within this GUI.
        void    setPos(const Vec2f& pos) { this->position = pos; }
        void    setY(float y) { this->position.y() = y; }
        void    setX(float x) { this->position.x() = x; }

        // Get the position of this GUI.
        Vec2f   getPos();
        float   getX() { return position.x(); }
        float   getY() { return position.y(); }

        // Return a list of all gui-elements attached to this gui
        std::vector<GUIElement*>& getGUIElements() { return guiElements; }

        // Sub gui's
        void addSubGUI(GUI* gui);
        void removeSubGUI(GUI* gui);

        // Update all GUI-Components in this GUI
        void update(float delta) override;

        void setIsActive(bool b) override { Component::setIsActive(b); for(auto& e : subGUIs) e->setIsActive(b); }


    private:
        GUI*                        parentGUI;
        std::vector<GUI*>           subGUIs;        // All sub-guis from this gui
        std::vector<GUIElement*>    guiElements;    // All GUI-Elements which belong to this GUI
        Vec2f                       position;       // The Position of the GUI. Used to manipulate the position of all elements.

        // Called from the GUI-Renderer when the window size has changed
        void onSizeChanged(float fbW, float fbH);
    };

}


#endif // !GUI_H_
