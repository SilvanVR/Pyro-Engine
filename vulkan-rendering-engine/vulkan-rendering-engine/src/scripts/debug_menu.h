#ifndef DEBUG_MENU_H_
#define DEBUG_MENU_H_


#include "vulkan-core/script_interface.hpp"
#include "interaction_script.h"

#include <map>

namespace Pyro
{

    class DebugMenuButton;

    struct Button
    {
        GUIButton*  button;
        GUIText*    text;
    };

    struct SliderGUI
    {
        GUI*       gui;
        GUISlider* slider;
    };

    class DebugMenu : public Node
    {
    public:
        DebugMenu(RenderingEngine* renderer, FontPtr font);
        ~DebugMenu();

        void update(float delta) override;

        void addButton(const std::string& name, std::function<void()> func, const std::string& parent = "");
        void addSliderButton(const std::string& name, std::function<void(float)> func, const std::string& parent = "");

        // Save the current state of the debug-menu
        void save();

    private:
        void onCurrentSceneLoad(Scene* newCurrentScene) override;
        void onCurrentSceneUnload() override;

        FontPtr     font;
        TexturePtr  buttonTexture;
        TexturePtr  sliderBackgroundTexture;

        GUI*        debugButtonGUI;
        GUIButton*  debugButton;
        GUIText*    debugButtonText;

        GUI*        mainGUI;

        // Used to disable the script when debug menu is open
        CInteract*  interactionScript;

        std::vector<DebugMenuButton>    buttons;
        std::vector<SliderGUI>          sliderGUIs;

        bool        isActive = false;

        int         numButtons = 0;
        int         numSliders = 0;

        GUIText*    numObjects;
        GUIText*    numLights;
        GUIText*    numTextures;
        GUIText*    runningTime;
        GUIText*    fps;
        CallbackID  fpsCallbackID;

        GUIText*    ramCurrentAllocated;
        GUIText*    ramTotalAllocated;
        GUIText*    gpuCurrentAllocated;

        GUIButton*  moveButton;
        GUIText*    moveButtonText;

        std::map<std::string, DebugMenuButton*> menuButtons;

        void enableSliderGUI(int num);
        void disableAllSecondGUIs();

        void init(RenderingEngine* renderer);

        std::vector<DebugMenuButton*>   elementsToRemove;
        std::vector<std::string>        menuButtonsToRemove;
    };


    class DebugMenuButton
    {

    public:
        GUI gui;

        //DebugMenuButton* parent;

        Button menuButton;

        int numButtons = 0;
        int numSliders = 0;

        // std::vector<DebugMenuButton>     buttons;
        std::map<std::string, DebugMenuButton*> children;
        std::vector<SliderGUI>                  sliderGUIs;

        ~DebugMenuButton()
        {
            for (auto& sliderGUI : sliderGUIs)
                delete sliderGUI.gui;
        }

        void disableGUI() {
            gui.setIsActive(false);
            for (auto& sliderGUI : sliderGUIs) { sliderGUI.gui->setIsActive(false); }
            for (auto& button : children) { button.second->disableGUI(); }
        }

        void toggleGUI() {
            gui.toggleActive();
            for (auto& sliderGUI : sliderGUIs) { sliderGUI.gui->setIsActive(false); }
            for (auto& button : children) { button.second->disableGUI(); }
        };

        void enableSliderGUI(int num)
        {
            for (auto& sliderGUI : sliderGUIs)
            {
                if (sliderGUIs[num].gui == sliderGUI.gui) { sliderGUIs[num].gui->toggleActive(); continue; }
                sliderGUI.gui->setIsActive(false);
            }
        }

        void disableChildrenGUIs(DebugMenuButton* child)
        {
            for (auto& button : children)
            {
                if (child == button.second) { button.second->toggleGUI(); continue; }
                button.second->disableGUI();
            }
        }

        void toggleChildrenGUIs()
        {
            for (auto& button : children)
                button.second->toggleGUI();
        }

        void addChildren(const std::string& name, DebugMenuButton* child)
        {
            children[name] = child;
            numButtons++;
        }
    };

}


#endif // !DEBUG_MENU_H_

