#include "gui.h"

#include "vulkan-core/sub_renderer/gui_renderer/gui_renderer.h"
#include "vulkan-core/vkTools/vk_tools.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------

    // Create an empty gui.
    GUI::GUI(bool isActive)
        : Component(isActive)
    {
        GUIRenderer::addGUI(this);
    }

    // Create an GUI with several gui-components. Add them with gui.add( { &comp1, &comp2, ... } );
    GUI::GUI(GUIElement* guiElem)
    {
        add(guiElem);
        GUIRenderer::addGUI(this);
    }

    // Create an GUI with several gui-components. Add them with gui.add( { &comp1, &comp2, ... } );
    GUI::GUI(const std::vector<GUIElement*>& guiElems)
    {
        add(guiElems);
        GUIRenderer::addGUI(this);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    GUI::~GUI()
    {
        if(parentGUI != nullptr)
            parentGUI->removeSubGUI(this);

        GUIRenderer::removeGUI(this);

        while(!guiElements.empty())
            delete guiElements.front();
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Update all GUI-Components in this GUI
    void GUI::update(float delta)
    {
        for (auto& subGUI : subGUIs)
            if(subGUI->isActive())
                subGUI->update(delta);

        for (auto& guiElem : guiElements)
            if(guiElem->isActive())
                guiElem->update(delta);
    }

    // Add a GUI-Component to this GUI.
    void GUI::add(GUIElement* guiComponent, bool renderFirst)
    {
        if (renderFirst)
            guiElements.insert(guiElements.begin(), guiComponent);
        else
            guiElements.push_back(guiComponent);
        guiComponent->setGUI(this);
    }

    // Add several GUI-Components to this GUI. Use with gui.add( { &comp1, &comp2, ... } );
    void GUI::add(const std::vector<GUIElement*>& guiElems)
    {
        for (auto& guiElem : guiElems)
        {
            guiElements.push_back(guiElem);
            guiElem->setGUI(this);
        }
    }

    // Remove a GUI-Element from this GUI.
    void GUI::remove(GUIElement* guiElement)
    {
        removeObjectFromList(guiElements, guiElement);
        guiElement->setGUI(nullptr);
    }

    // Remove several GUI-Components from this GUI.
    void GUI::remove(const std::vector<GUIElement*>& guiComps)
    {
        for (auto& guiElem : guiComps)
        {
            removeObjectFromList(guiElements, guiElem);
            guiElem->setGUI(nullptr);
        }
    }
    
    // Set the color for all gui-elements in this GUI.
    void GUI::setColor(const Color& color)
    {
        for (auto& guiElem : guiElements)
            guiElem->setColor(color);
    }

    // Set the alpha value for the whole GUI.
    void GUI::setAlpha(float alpha)
    {
        for (auto& guiElem : guiElements)
            guiElem->setAlpha(alpha);
    }

    // Return the position from this gui
    Vec2f GUI::getPos() 
    { 
        if(parentGUI != nullptr)
            return position + parentGUI->getPos();

        return position;
    }

    // Set this GUI on top of all GUI's
    void GUI::onTop()
    {
        GUIRenderer::removeGUI(this);
        GUIRenderer::addGUI(this);
    }

    // Add a sub gui to this gui, which inherits the position from the parent gui
    void GUI::addSubGUI(GUI* gui) 
    {
        gui->parentGUI = this;
        subGUIs.push_back(gui); 
    }
    
    // Remove a sub-gui from this gui
    void GUI::removeSubGUI(GUI* gui)
    { 
        gui->parentGUI = nullptr;
        removeObjectFromList(subGUIs, gui);
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Called from the GUI-Renderer when the window size has changed
    void GUI::onSizeChanged(float fbW, float fbH)
    {
        // for(auto& guiComp : guiComponents)
            // Nothing to do yet
    }

}