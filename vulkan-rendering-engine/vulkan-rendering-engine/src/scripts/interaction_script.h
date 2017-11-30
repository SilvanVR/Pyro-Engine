#ifndef INTERACTION_SCRIPT_H_
#define INTERACTION_SCRIPT_H_

#include "vulkan-core/script_interface.hpp"


namespace Pyro
{

    class CInteract : public Component
    {

    public:
        CInteract(float rayDistance);
        ~CInteract();

        void update(float delta) override;

        const Node* getSelectedNode() const { return selectedObject; }

    private:
        Node* selectedObject;

        // Stores the distance from the ray to the object when selected
        float currentDistance;

        // Max distance of the ray
        float maxRayDistance;

        // GUI for displaying the properties of the selected object
        GUI componentGUI;

        // Background texture for the buttons
        TexturePtr backgroundTexture;

        // Background-Image for the component-GUI
        GUIImage* backgroundImage;

        // Text for the Transform of the selected Object
        GUIText* objectNameText;
        GUIText* objectPositionText;
        GUIText* objectRotationText;
        GUIText* objectScaleText;

    };


}

#endif // !INTERACTION_SCRIPT_H_

