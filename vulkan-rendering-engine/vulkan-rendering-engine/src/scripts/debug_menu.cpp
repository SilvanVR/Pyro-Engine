#include "debug_menu.h"

#include "time/time.h"
#include "Input/input.h"


// DONT READ THIS CODE. ITS HORRIBLE AND WAS MADE PRETTY FAST

namespace Pyro
{
    float buttonWidth = 100;
    float buttonHeight = 50;
    float paddingBetweenButtons = 5;
    float sliderLength = 200;

#define tVal 0.95f
#define bVal 0.5f
    static Color textColor(tVal, tVal, tVal,1);
    static Color buttonColor(bVal, bVal, bVal, 0.9f);

    static Color textHoverColor(0,0,0,1);
    static Color buttonHoverColor(1,1,1, 0.8f);

    DebugMenu::DebugMenu(RenderingEngine* renderer, FontPtr _font)
        : Node("DebugMenu", Node::EType::Static), font(_font)
    {
        buttonTexture = TEXTURE({ "/textures/button4.dds" });
        sliderBackgroundTexture = TEXTURE({ "/textures/slider.dds" });

        this->makeGlobal();

        debugButton = new GUIButton(buttonTexture, Rectf(-buttonWidth - 3.0f, 3.0f, buttonWidth, buttonHeight), buttonColor, Anchor::TopRight);

        debugButtonText = new GUIText("Debug", Vec2f(0, font->getTextHeight("Debug") / 2.0f), font, textColor, Vec2f(0.8f, 0.8f), Anchor::Center, TextAlign::CENTER);
        debugButton->addChild(debugButtonText);

        //debugButton->attachFunc(GUIButton::BUTTONPRESSED, [&] { mainGUI->toggleActive(); disableAllSecondGUIs(); isActive = !isActive; });
        //debugButton->attachFunc(GUIButton::MOUSEOVER, [&] { debugButton->setColor(buttonHoverColor); debugButtonText->setColor(textHoverColor); });
        //debugButton->attachFunc(GUIButton::MOUSEOUT, [&] { debugButton->setColor(buttonColor); debugButtonText->setColor(textColor); });

        float fontScale = 1.0f;
        numObjects          = new GUIText("TEST", Vec2f(5, 60), font, Color::RED, Vec2f(fontScale, fontScale));
        numLights           = new GUIText("TEST", Vec2f(5, 90), font, Color::GREEN, Vec2f(fontScale, fontScale));
        numTextures         = new GUIText("TEST", Vec2f(5, 120), font, Color::WHITE, Vec2f(fontScale, fontScale));
        runningTime         = new GUIText("TEST", Vec2f(0, -5), font, Color(0, 1, 1), Vec2f(fontScale, fontScale), Anchor::BottomLeft);
        ramCurrentAllocated = new GUIText("TEST", Vec2f(5, 150), font, Color::WHITE, Vec2f(fontScale, fontScale));
        ramTotalAllocated   = new GUIText("TEST", Vec2f(5, 180), font, Color::WHITE, Vec2f(fontScale, fontScale));
        gpuCurrentAllocated = new GUIText("TEST", Vec2f(5, 210), font, Color::WHITE, Vec2f(fontScale, fontScale));
        fps                 = new GUIText("FPS", Vec2f(5, 30), Color(1, 0, 1, 1));

        fpsCallbackID = Time::setInterval([=] {
            std::string fpsString = "FPS: " + std::to_string(Time::getFPS()) + " (" + std::to_string(1000.0f / Time::getFPS()) + " ms)";
            fps->setText(fpsString);
        }, 1000);

        // Moving Button
        //moveButton = new GUIButton(buttonTexture, Rectf(200, 100), Color::WHITE, Anchor::CenterLeft);
        //moveButtonText = new GUIText("Klick!", Vec2f(0.0f, font->getTextHeight("Klick!") / 2.0f), font, Color::BLACK, Vec2f(0.8f, 0.8f), Anchor::Center, TextAlign::CENTER);
        //moveButton->addChild(moveButtonText);
        //
        //moveButton->attachFunc(GUIButton::BUTTONPRESSED, [&] { moveButtonText->setText("PRESSED"); moveButton->setColor(Color::BLUE); });
        //moveButton->attachFunc(GUIButton::BUTTONDOWN, [&] { moveButton->setLocalPos(moveButton->getLocalPos() + (Vec2f)Input::getDeltaMousePos()); });
        //moveButton->attachFunc(GUIButton::MOUSEOVER, [&] { moveButtonText->setText("MOUSEOVER"); moveButton->setColor(Color::GREEN); });
        //moveButton->attachFunc(GUIButton::MOUSEOUT, [&] { moveButtonText->setText("MOUSEOUT"); moveButton->setColor(Color::WHITE);  });

        debugButtonGUI = new GUI();
        debugButtonGUI->add({ debugButton, debugButtonText, fps });

        addComponent(debugButtonGUI);

        mainGUI = new GUI(false);
        mainGUI->add({ numObjects, numLights, numTextures, runningTime, ramCurrentAllocated, ramTotalAllocated, gpuCurrentAllocated /*, moveButton, moveButtonText*/ });

        addComponent(mainGUI);

        init(renderer);
    }

    DebugMenu::~DebugMenu()
    {
        Time::clearCallback(fpsCallbackID);

        for (auto& sliderGUI : sliderGUIs)
            delete sliderGUI.gui;

        for (auto& menuButton : menuButtons)
            delete menuButton.second;
    }


    void DebugMenu::onCurrentSceneLoad(Scene* newCurrentScene)
    {
        Node* interactionNode = SceneManager::getCurrentScene()->findNode("InteractionMaster");

        if (interactionNode != nullptr)
            interactionScript = interactionNode->getComponent<CInteract>();

        debugButton->attachFunc(GUIButton::BUTTONPRESSED, [&] { mainGUI->toggleActive(); disableAllSecondGUIs(); isActive = !isActive; });
        debugButton->attachFunc(GUIButton::MOUSEOVER, [&] { debugButton->setColor(buttonHoverColor); debugButtonText->setColor(textHoverColor);
                                                            if (interactionScript != nullptr) interactionScript->setIsActive(false); });
        debugButton->attachFunc(GUIButton::MOUSEOUT, [&] { debugButton->setColor(buttonColor); debugButtonText->setColor(textColor);
                                                            if (!isActive) { if (interactionScript != nullptr) interactionScript->setIsActive(true); } });
    }

    void DebugMenu::onCurrentSceneUnload()
    {
        interactionScript = nullptr;
        debugButton->detachAllFuncs(GUIButton::Event::BUTTONPRESSED);
        debugButton->detachAllFuncs(GUIButton::Event::MOUSEOVER);
        debugButton->detachAllFuncs(GUIButton::Event::MOUSEOUT);

        for (auto& e : elementsToRemove)
        {
            numButtons--;
            delete e->menuButton.button;
            delete e->menuButton.text;
        }
        elementsToRemove.clear();

        for (auto& b : menuButtonsToRemove)
        {
            delete menuButtons[b];
            menuButtons.erase(b);
        }
        menuButtonsToRemove.clear();
    }

    void DebugMenu::save()
    {
        elementsToRemove.clear();
        menuButtonsToRemove.clear();
    }

    // Add a button to the debug-menu
    void DebugMenu::addButton(const std::string& name, std::function<void()> func, const std::string& parent)
    {
        GUIButton*  button;
        GUIText*    text;

        std::string hashMapName = "";

        // Calculate appropriate scale
        int textWidth = font->getTextWidth(name);
        float textScale = buttonWidth / textWidth;
        if (textScale > 1.0f) textScale = 1.0f; // Scale the text only smaller, not bigger

        if (parent == "")
        {
            hashMapName = name;

            // Calculate Position
            float buttonY = (numButtons + 1) * buttonHeight + paddingBetweenButtons * (numButtons + 1);
            float buttonX = 0.0f;

            // Create Button & Text
            button = new GUIButton(buttonTexture, Rectf(buttonX, buttonY, buttonWidth, buttonHeight), buttonColor);
            text = new GUIText(name, Vec2f(0, font->getTextHeight(name) * textScale / 2.0f), font, textColor, Vec2f(textScale, textScale), Anchor::Center, TextAlign::CENTER);
            text->setInheritScale(false);
            button->addChild(text);

            // Create a container class for that
            DebugMenuButton* newMenuButton = new DebugMenuButton;
            newMenuButton->gui.setIsActive(false);
            mainGUI->addSubGUI(&newMenuButton->gui);
            newMenuButton->menuButton = { button, text };

            menuButtons[name] = newMenuButton;
            numButtons++;

            // Attach function to the button when he is pressed. Disable all Slider-GUIs and enable the gui attached to the button
            button->attachFunc(GUIButton::BUTTONPRESSED, [&, name] { disableAllSecondGUIs(); menuButtons[name]->toggleGUI(); });

            // Add the button as a child of the debugButton
            debugButton->addChild(button);
            // Add the GUIElements to the mainGUI, otherwise they wont be rendered
            mainGUI->add({ button, text });
            elementsToRemove.push_back(newMenuButton);
            menuButtonsToRemove.push_back(name);
        }
        else
        {
            hashMapName = parent + "-" + name;

            // Calculate Position
            float buttonY = (menuButtons[parent]->numButtons) * buttonHeight + paddingBetweenButtons * (menuButtons[parent]->numButtons);
            float buttonX = -buttonWidth - 4;

            // Create Button & Text
            button = new GUIButton(buttonTexture, Rectf(buttonX, buttonY, buttonWidth, buttonHeight), buttonColor);
            text = new GUIText(name, Vec2f(0, font->getTextHeight(name) * textScale / 2.0f), font, textColor, Vec2f(textScale, textScale), Anchor::Center, TextAlign::CENTER);
            text->setInheritScale(false);
            button->addChild(text);

            // Create a container class for that
            DebugMenuButton* newMenuButton = new DebugMenuButton;
            newMenuButton->gui.setIsActive(false);
            menuButtons[parent]->gui.addSubGUI(&newMenuButton->gui);
            newMenuButton->menuButton = { button, text };

            menuButtons[hashMapName] = newMenuButton;

            // Attach function to the button when he is pressed. Disable all Slider-GUIs and enable the gui attached to the button
            button->attachFunc(GUIButton::BUTTONPRESSED, [&, parent, newMenuButton] { menuButtons[parent]->disableChildrenGUIs(newMenuButton); });

            button->setParent(menuButtons[parent]->menuButton.button);

            menuButtons[parent]->addChildren(hashMapName, newMenuButton);

            menuButtons[parent]->gui.add({ button, text });

            menuButtonsToRemove.push_back(hashMapName);
        }

        // Add some visual effect when hovering over the button
        button->attachFunc(GUIButton::MOUSEOVER, [&, hashMapName] { menuButtons[hashMapName]->menuButton.button->setColor(buttonHoverColor); menuButtons[hashMapName]->menuButton.text->setColor(textHoverColor); });
        button->attachFunc(GUIButton::MOUSEOUT, [&, hashMapName] { menuButtons[hashMapName]->menuButton.button->setColor(buttonColor); menuButtons[hashMapName]->menuButton.text->setColor(textColor); });

        // Attach the lambda-function to the button
        if (func != nullptr)
            button->attachFunc(GUIButton::BUTTONPRESSED, func);
    }

    // Add a slider-button to the debug-menu
    void DebugMenu::addSliderButton(const std::string& name, std::function<void(float)> func, const std::string& parent)
    {
        std::string hashMapName = "";

        GUIButton*  button;
        GUIText*    text;

        // Calculate appropriate scale
        int textWidth = font->getTextWidth(name);
        float textScale = buttonWidth / textWidth;
        if (textScale > 1.0f) textScale = 1.0f;

        if (parent == "")
        {
            hashMapName = name;

            // Calculate Position
            float buttonY = (numButtons + 1) * buttonHeight + paddingBetweenButtons * (numButtons + 1);
            float buttonX = 0.0f;

            // Create Button & Text
            button = new GUIButton(buttonTexture, Rectf(buttonX, buttonY, buttonWidth, buttonHeight), buttonColor);
            text = new GUIText(name, Vec2f(0, font->getTextHeight(name) * textScale / 2.0f), font, textColor, Vec2f(textScale, textScale), Anchor::Center, TextAlign::CENTER);
            text->setInheritScale(false);
            button->addChild(text);

            // Create a container class for that
            DebugMenuButton* newMenuButton = new DebugMenuButton;
            newMenuButton->gui.setIsActive(false);
            mainGUI->addSubGUI(&newMenuButton->gui);
            newMenuButton->menuButton = { button, text };

            menuButtons[name] = newMenuButton;
            numButtons++;
            elementsToRemove.push_back(newMenuButton);
            menuButtonsToRemove.push_back(name);

            // Create the Slider
            float padding = 4; // padding in pixels between the slider and the button-row on the right side
            GUISlider* slider = new GUISlider(sliderBackgroundTexture, sliderLength, buttonTexture, Rectf(-50 - padding, 0, 50, 50), Color::WHITE, GUISlider::VERTICAL, Anchor::TopLeft);
            slider->attachFunc(func);
            button->addChild(slider);

            // Create a new GUI for the slider only
            GUI* gui = new GUI(false);
            newMenuButton->gui.addSubGUI(gui);
            gui->add(slider);

            // Add it to the list
            sliderGUIs.push_back({ gui, slider });

            // Attach function to the button which toggles the Slider-GUI
            int num = numSliders++; // capture by value, not by reference
            button->attachFunc(GUIButton::BUTTONPRESSED, [&, num] { enableSliderGUI(num); });

            // Add the button as a child of the debugButton
            debugButton->addChild(button);

            // Add the GUIElements to the mainGUI, otherwise they wont be rendered
            mainGUI->add({ button, text });

        }
        else
        {
            hashMapName = parent + "-" + name;

            // Calculate Position
            float buttonY = (menuButtons[parent]->numButtons) * buttonHeight + paddingBetweenButtons * (menuButtons[parent]->numButtons);
            float buttonX = -buttonWidth - 4;

            // Create Button & Text
            button = new GUIButton(buttonTexture, Rectf(buttonX, buttonY, buttonWidth, buttonHeight), buttonColor);
            text = new GUIText(name, Vec2f(0, font->getTextHeight(name) * textScale / 2.0f), font, textColor, Vec2f(textScale, textScale), Anchor::Center, TextAlign::CENTER);
            text->setInheritScale(false);
            button->addChild(text);

            // Create a container class for that
            DebugMenuButton* newMenuButton = new DebugMenuButton;
            newMenuButton->gui.setIsActive(false);
            menuButtons[parent]->gui.addSubGUI(&newMenuButton->gui);
            newMenuButton->menuButton = { button, text };

            menuButtons[hashMapName] = newMenuButton;
            menuButtonsToRemove.push_back(hashMapName);

            // Create the Slider
            float padding = 4; // padding in pixels between the slider and the button-row on the right side
            GUISlider* slider = new GUISlider(sliderBackgroundTexture, sliderLength, buttonTexture, Rectf(-50 - padding, 0, 50, 50), Color::WHITE, GUISlider::VERTICAL, Anchor::TopLeft);
            slider->attachFunc(func);
            button->addChild(slider);

            // Create a new GUI for the slider only
            GUI* gui = new GUI(false);
            newMenuButton->gui.addSubGUI(gui);
            gui->add(slider);

            // Add it to the list
            newMenuButton->sliderGUIs.push_back({ gui, slider });

            // Attach function to the button which toggles the Slider-GUI
            int num = menuButtons[hashMapName]->numSliders++; // capture by value, not by reference
            button->attachFunc(GUIButton::BUTTONPRESSED, [&, parent, hashMapName, newMenuButton, num] { menuButtons[parent]->disableChildrenGUIs(newMenuButton); menuButtons[hashMapName]->enableSliderGUI(num); });

            // 
            button->setParent(menuButtons[parent]->menuButton.button);

            //
            menuButtons[parent]->addChildren(hashMapName, newMenuButton);

            // 
            menuButtons[parent]->gui.add({ button, text });
        }

        // Add some visual effect when hovering over the button
        button->attachFunc(GUIButton::MOUSEOVER, [&, hashMapName] { menuButtons[hashMapName]->menuButton.button->setColor(buttonHoverColor); menuButtons[hashMapName]->menuButton.text->setColor(textHoverColor); });
        button->attachFunc(GUIButton::MOUSEOUT, [&, hashMapName] { menuButtons[hashMapName]->menuButton.button->setColor(buttonColor); menuButtons[hashMapName]->menuButton.text->setColor(textColor); });
    }


    // Update Text-Objects
    void DebugMenu::update(float delta)
    {
        Node::update(delta);

        const Scene* currentScene = SceneManager::getCurrentScene();
        Camera* currentCamera = RenderingEngine::getCamera();

        numObjects->setText("Num Objects: " + std::to_string(currentScene->getRenderables().size()) + 
                            " (Visible: " + std::to_string(currentCamera->getLastTimeRendered().size()) + ")");
        numLights->setText("Num Lights: " + std::to_string(currentScene->getLights().size()) + 
                           " (Visible: " + std::to_string(currentCamera->getLastTimeRenderedLights().size()) + ")");
        numTextures->setText("Num Textures: " + std::to_string(NUM_TEXTURES));
        runningTime->setText("Running Time: " + std::to_string(Time::getTotalRunningTimeInSeconds()) + "s");
        ramCurrentAllocated->setText("Current Allocated: " + MemoryManager::bytesToString(MemoryManager::getMemoryInfo().currentAllocated));
        ramTotalAllocated->setText("Total RAM Allocated: " + MemoryManager::bytesToString(MemoryManager::getMemoryInfo().totalAllocated));

        std::string gpuMemString = "GPU Mem Allocated: " + MemoryManager::bytesToString(VMM::getMemoryInfo().currentAllocated);
        std::string nearestPercentage = toStringWithPrecision(VMM::getMemoryInfo().percentageUsed, 3);
        gpuCurrentAllocated->setText(gpuMemString + " ("+ nearestPercentage +"% Used)");
    }


    // Enable a given Slider-GUI per index
    void DebugMenu::enableSliderGUI(int num)
    {
        for (auto& menuButton : menuButtons)
            menuButton.second->disableGUI();

        for (auto& sliderGUI : sliderGUIs)
        {
            if (sliderGUIs[num].gui == sliderGUI.gui) continue;
            sliderGUI.gui->setIsActive(false);
        }

        sliderGUIs[num].gui->toggleActive();
    }

    void DebugMenu::disableAllSecondGUIs()
    {
        for (auto& menuButton : menuButtons)
            menuButton.second->disableGUI();

        for (auto& sliderGUI : sliderGUIs)
            sliderGUI.gui->setIsActive(false);
    }


    void DebugMenu::init(RenderingEngine* renderer)
    {
        addButton("Settings", nullptr);
        addButton("Rendering-Mode", nullptr, "Settings");
        addButton("Wireframe", [=] { renderer->setRenderingMode(ERenderingMode::WIREFRAME); }, "Settings-Rendering-Mode");
        addButton("Solid", [=] { renderer->setRenderingMode(ERenderingMode::SOLID); }, "Settings-Rendering-Mode");
        addButton("Lit", [=] { renderer->setRenderingMode(ERenderingMode::LIT); }, "Settings-Rendering-Mode");
        addButton("ToggleNormalMaps", [=] { renderer->toggleNormalMapRendering(); }, "Settings-Rendering-Mode-Lit");
        addButton("ToggleDispMaps", [=] { renderer->toggleDispMapRendering(); }, "Settings-Rendering-Mode-Lit");
        addButton("Albedo", [=] { renderer->toggleRenderAlbedo(); }, "Settings-Rendering-Mode-Lit");
        addButton("Normal", [=] { renderer->toggleRenderNormals(); }, "Settings-Rendering-Mode-Lit");

        addButton("PostProcessing", nullptr, "Settings");
        addButton("Toggle Post-Process", [=] { renderer->togglePostProcessing(); }, "Settings-PostProcessing");
        addSliderButton("Resolution Mod", [=](float value) { renderer->setResolutionMod(value); }, "Settings-PostProcessing");
        if (SHADER_EXISTS("FXAA"))
        {
            addButton("FXAA", nullptr, "Settings-PostProcessing");
            addButton("Toggle", [&] { SHADER("FXAA")->toggleActive(); }, "Settings-PostProcessing-FXAA");
            addSliderButton("SpanMax", [&](float value) { SHADER("FXAA")->setFloat("spanMax", value * 10); }, "Settings-PostProcessing-FXAA");
            addSliderButton("ReduceMin", [&](float value) { SHADER("FXAA")->setFloat("reduceMin", value / 10); }, "Settings-PostProcessing-FXAA");
            addSliderButton("ReduceMax", [&](float value) { SHADER("FXAA")->setFloat("reduceMul", value); }, "Settings-PostProcessing-FXAA");
        }
        if (SHADER_EXISTS("Tonemap"))
        {
            addButton("Tonemap", nullptr, "Settings-PostProcessing");
            addButton("Toggle", [&] { SHADER("Tonemap")->toggleActive(); }, "Settings-PostProcessing-Tonemap");
            addSliderButton("Exposure", [&](float value) { SHADER("Tonemap")->setFloat("exposure", 5 * value); }, "Settings-PostProcessing-Tonemap");
            addSliderButton("Gamma", [&](float value) { SHADER("Tonemap")->setFloat("gamma", 2.2f * value); }, "Settings-PostProcessing-Tonemap");
        }
        if (SHADER_EXISTS("HDRBloom"))
        {
            addButton("HDRBloom", nullptr, "Settings-PostProcessing");
            addButton("Toggle", [&] { SHADER("HDRBloom")->toggleActive(); }, "Settings-PostProcessing-HDRBloom");
            addSliderButton("Combine Strength", [&](float value) { SHADER("HDRBloom")->setFloat("combineStrength", value * 2); }, "Settings-PostProcessing-HDRBloom");
            addSliderButton("Brightness Threshold", [&](float value) { SHADER("HDRBloom#BrightFilter")->setFloat("brightnessThreshold", value * 10); }, "Settings-PostProcessing-HDRBloom");
            addSliderButton("Blur Strength", [&](float value) {
                SHADER("HDRBloom#HBlur1")->setFloat("blurStrength", value * 10);
                SHADER("HDRBloom#VBlur1")->setFloat("blurStrength", value * 10);
            }, "Settings-PostProcessing-HDRBloom");
        }
        if (SHADER_EXISTS("DepthOfField"))
        {
            addButton("DOA", nullptr, "Settings-PostProcessing");
            addButton("Toggle", [&] { SHADER("DepthOfField")->toggleActive(); }, "Settings-PostProcessing-DOA");
            addSliderButton("Focus-Treshold", [&](float value) { SHADER("DepthOfField")->setFloat("focusThreshold", value * 500.0f); }, "Settings-PostProcessing-DOA");
            addSliderButton("Transition-Distance", [&](float value) { SHADER("DepthOfField")->setFloat("transitionDistance", value * 100.0f); }, "Settings-PostProcessing-DOA");
        }

        bool lightShaftExists = SHADER_EXISTS("LightShafts");
        bool cameraDirtExists = SHADER_EXISTS("CameraDirt");
        bool lensFlaresExists = SHADER_EXISTS("LensFlares");
        if (lightShaftExists || cameraDirtExists || lensFlaresExists)
        {
            addButton("Sun", nullptr, "Settings-PostProcessing");
            if (lightShaftExists)
            {
                addButton("LightShafts", nullptr, "Settings-PostProcessing-Sun");
                addButton("Toggle", [=] { SHADER("LightShafts")->toggleActive(); }, "Settings-PostProcessing-Sun-LightShafts");
                addSliderButton("Scale", [=](float value) { SHADER("LightShafts#RadialBlur")->setFloat("radialBlurScale", value); }, "Settings-PostProcessing-Sun-LightShafts");
                addSliderButton("Strength", [=](float value) { SHADER("LightShafts#RadialBlur")->setFloat("radialBlurStrength", value * 2); }, "Settings-PostProcessing-Sun-LightShafts");
            }
            if (cameraDirtExists)
            {
                addButton("CameraDirt", nullptr, "Settings-PostProcessing-Sun");
                addButton("Toggle", [=] { SHADER("CameraDirt")->toggleActive(); }, "Settings-PostProcessing-Sun-CameraDirt");
            }
            if (lensFlaresExists)
            {
                addButton("LensFlares", nullptr, "Settings-PostProcessing-Sun");
                addButton("Toggle", [=] { SHADER("LensFlares")->toggleActive(); }, "Settings-PostProcessing-Sun-LensFlares");
            }
        }
        addButton("UseIBL", [=] { renderer->setUseIBL(renderer->isUsingIBL() ? false : true); }, "Settings");
        addSliderButton("Ambient", [=](float value) { renderer->setAmbientIntensity(value * 2); }, "Settings");
        addSliderButton("Specular", [=](float value) { renderer->setEnvMapSpecularIntensity(value * 2); }, "Settings");
        addSliderButton("Time-Scale", [=](float value) { renderer->setTimeScale(value * 8 - 4); }, "Settings");
        addSliderButton("Alpha-Discarding", [=](float value) { renderer->setAlphaDiscardThreshold(value); }, "Settings");
        addButton("Toggle Type", [] {
            for (auto obj : SceneManager::getCurrentScene()->getRenderables())
                obj->toggleType();
        }, "Settings");

        addButton("Camera", nullptr);
        addButton("Ortho", [=] { RenderingEngine::getCamera()->setRenderingMode(Camera::ORTHOGRAPHIC); }, "Camera");
        addButton("Perspective", [=] { RenderingEngine::getCamera()->setRenderingMode(Camera::PERSPECTIVE); }, "Camera");
        addSliderButton("FOV", [=](float value) { RenderingEngine::getCamera()->setFOV(value * 180); }, "Camera");
        addSliderButton("zFar", [=](float value) { RenderingEngine::getCamera()->setZFar(value * 1000.0f); }, "Camera");
        addSliderButton("zNear", [=](float value) { RenderingEngine::getCamera()->setZNear(value * 100.0f); }, "Camera");
        addSliderButton("Left", [=](float value) { RenderingEngine::getCamera()->setLeft(-value * 100); }, "Camera");
        addSliderButton("Right", [=](float value) { RenderingEngine::getCamera()->setRight(value * 100); }, "Camera");
        addSliderButton("Top", [=](float value) { RenderingEngine::getCamera()->setTop(value * 100); }, "Camera");
        addSliderButton("Bottom", [=](float value) { RenderingEngine::getCamera()->setBottom(-value * 100); }, "Camera");

        if (SHADER_EXISTS("FOG"))
        {
            addButton("FOG", nullptr);
            addButton("Toggle", [=] { renderer->toggleFOG(); }, "FOG");
            addSliderButton("Density", [=](float value) { renderer->setFOGDensity(value * 0.01f); }, "FOG");
            addSliderButton("Gradient", [=](float value) { renderer->setFOGGradient(value * 10.0f + 1); }, "FOG");
            addButton("Color", nullptr, "FOG");
            addSliderButton("Red", [=](float value) { const Color& fogColor = renderer->getFogColor();
            renderer->setFogColor(Color(value, fogColor.g(), fogColor.b())); }, "FOG-Color");
            addSliderButton("Green", [=](float value) {const Color& fogColor = renderer->getFogColor();
            renderer->setFogColor(Color(fogColor.r(), value, fogColor.b()));  }, "FOG-Color");
            addSliderButton("Blue", [=](float value) {const Color& fogColor = renderer->getFogColor();
            renderer->setFogColor(Color(fogColor.r(), fogColor.g(), value)); }, "FOG-Color");
        }

        this->save();
    }

}





//
//void Application::initDebugMenu()
//{
//    DebugMenu* debugMenu = new DebugMenu(new Font("DebugFont", "/fonts/arial/arial.ttf", 24, Object::GLOBAL));
//    {
//        debugMenu->addButton("Settings", nullptr);
//        debugMenu->addButton("Rendering-Mode", nullptr, "Settings");
//        debugMenu->addButton("Wireframe", [=] { renderer.setRenderingMode(ERenderingMode::WIREFRAME); }, "Settings-Rendering-Mode");
//        debugMenu->addButton("Solid", [=] { renderer.setRenderingMode(ERenderingMode::SOLID); }, "Settings-Rendering-Mode");
//        debugMenu->addButton("Lit", [=] { renderer.setRenderingMode(ERenderingMode::LIT); }, "Settings-Rendering-Mode");
//        debugMenu->addButton("ToggleNormalMaps", [=] { renderer.toggleNormalMapRendering(); }, "Settings-Rendering-Mode-Lit");
//        debugMenu->addButton("ToggleDispMaps", [=] { renderer.toggleDispMapRendering(); }, "Settings-Rendering-Mode-Lit");
//        debugMenu->addButton("Albedo", [=] { renderer.toggleRenderAlbedo(); }, "Settings-Rendering-Mode-Lit");
//        debugMenu->addButton("Normal", [=] { renderer.toggleRenderNormals(); }, "Settings-Rendering-Mode-Lit");
//
//        debugMenu->addButton("UseIBL", [=] { renderer.setUseIBL(renderer.isUsingIBL() ? false : true); }, "Settings");
//        debugMenu->addSliderButton("Ambient", [=](float value) { renderer.setAmbientIntensity(value * 2); }, "Settings");
//        debugMenu->addSliderButton("Time-Scale", [=](float value) { renderer.setTimeScale(value * 8 - 4); }, "Settings");
//        debugMenu->addSliderButton("Alpha-Discarding", [=](float value) { renderer.setAlphaDiscardThreshold(value); }, "Settings");
//        debugMenu->addButton("Toggle Type", [] {
//            for (auto obj : SceneManager::getCurrentScene()->getRenderables())
//                obj->toggleType();
//        }, "Settings");
//        debugMenu->addButton("PostProcessing", nullptr, "Settings");
//        debugMenu->addButton("Toggle Post-Process", [=] { renderer.togglePostProcessing(); }, "Settings-PostProcessing");
//        debugMenu->addSliderButton("Resolution Mod", [=](float value) { renderer.setResolutionMod(value); }, "Settings-PostProcessing");
//        if (SHADER_EXISTS("FXAA"))
//        {
//            debugMenu->addButton("FXAA", nullptr, "Settings-PostProcessing");
//            debugMenu->addButton("Toggle", [&] { SHADER("FXAA")->toggleActive(); }, "Settings-PostProcessing-FXAA");
//            debugMenu->addSliderButton("SpanMax", [&](float value) { SHADER("FXAA")->setFloat("spanMax", value * 10); }, "Settings-PostProcessing-FXAA");
//            debugMenu->addSliderButton("ReduceMin", [&](float value) { SHADER("FXAA")->setFloat("reduceMin", value / 10); }, "Settings-PostProcessing-FXAA");
//            debugMenu->addSliderButton("ReduceMax", [&](float value) { SHADER("FXAA")->setFloat("reduceMul", value); }, "Settings-PostProcessing-FXAA");
//        }
//        if (SHADER_EXISTS("Tonemap"))
//        {
//            debugMenu->addButton("Tonemap", nullptr, "Settings-PostProcessing");
//            debugMenu->addButton("Toggle", [&] { SHADER("Tonemap")->toggleActive(); }, "Settings-PostProcessing-Tonemap");
//            debugMenu->addSliderButton("Exposure", [&](float value) { SHADER("Tonemap")->setFloat("exposure", 5 * value); }, "Settings-PostProcessing-Tonemap");
//            debugMenu->addSliderButton("Gamma", [&](float value) { SHADER("Tonemap")->setFloat("gamma", 2.2f * value); }, "Settings-PostProcessing-Tonemap");
//        }
//        if (SHADER_EXISTS("HDRBloom"))
//        {
//            debugMenu->addButton("HDRBloom", nullptr, "Settings-PostProcessing");
//            debugMenu->addButton("Toggle", [&] { SHADER("HDRBloom")->toggleActive(); }, "Settings-PostProcessing-HDRBloom");
//            debugMenu->addSliderButton("Combine Strength", [&](float value) { SHADER("HDRBloom")->setFloat("combineStrength", value * 2); }, "Settings-PostProcessing-HDRBloom");
//            debugMenu->addSliderButton("Brightness Threshold", [&](float value) { SHADER("HDRBloom#BrightFilter")->setFloat("brightnessThreshold", value * 10); }, "Settings-PostProcessing-HDRBloom");
//            debugMenu->addSliderButton("Blur Strength", [&](float value) {
//                SHADER("HDRBloom#HBlur1")->setFloat("blurStrength", value * 10);
//                SHADER("HDRBloom#VBlur1")->setFloat("blurStrength", value * 10);
//            }, "Settings-PostProcessing-HDRBloom");
//        }
//        if (SHADER_EXISTS("DepthOfField"))
//        {
//            debugMenu->addButton("DOA", nullptr, "Settings-PostProcessing");
//            debugMenu->addButton("Toggle", [&] { SHADER("DepthOfField")->toggleActive(); }, "Settings-PostProcessing-DOA");
//            debugMenu->addSliderButton("Focus-Treshold", [&](float value) { SHADER("DepthOfField")->setFloat("focusThreshold", value * 500.0f); }, "Settings-PostProcessing-DOA");
//            debugMenu->addSliderButton("Transition-Distance", [&](float value) { SHADER("DepthOfField")->setFloat("transitionDistance", value * 100.0f); }, "Settings-PostProcessing-DOA");
//        }
//
//        debugMenu->addButton("Scenes", nullptr);
//        debugMenu->addButton("Cube-Scene", [=]() { SceneManager::switchScene(new TestScene2()); }, "Scenes");
//        debugMenu->addButton("Empty Scene", [=]() { SceneManager::switchScene(new EmptyScene()); }, "Scenes");
//
//        Camera* cam = RenderingEngine::getCamera();
//        debugMenu->addButton("Camera", nullptr);
//        debugMenu->addButton("Ortho", [=] { cam->setRenderingMode(Camera::ORTHOGRAPHIC); }, "Camera");
//        debugMenu->addButton("Perspective", [=] { cam->setRenderingMode(Camera::PERSPECTIVE); }, "Camera");
//        debugMenu->addSliderButton("FOV", [=](float value) { cam->setFOV(value * 180); }, "Camera");
//        debugMenu->addSliderButton("zFar", [=](float value) { cam->setZFar(value * 1000.0f); }, "Camera");
//        debugMenu->addSliderButton("zNear", [=](float value) { cam->setZNear(value * 100.0f); }, "Camera");
//        debugMenu->addSliderButton("Left", [=](float value) { cam->setLeft(-value * 100); }, "Camera");
//        debugMenu->addSliderButton("Right", [=](float value) { cam->setRight(value * 100); }, "Camera");
//        debugMenu->addSliderButton("Top", [=](float value) { cam->setTop(value * 100); }, "Camera");
//        debugMenu->addSliderButton("Bottom", [=](float value) { cam->setBottom(-value * 100); }, "Camera");
//
//        if (SHADER_EXISTS("FOG"))
//        {
//            debugMenu->addButton("FOG", nullptr);
//            debugMenu->addButton("Toggle", [=] { renderer.toggleFOG(); }, "FOG");
//            debugMenu->addSliderButton("Density", [=](float value) { renderer.setFOGDensity(value * 0.01f); }, "FOG");
//            debugMenu->addSliderButton("Gradient", [=](float value) { renderer.setFOGGradient(value * 10.0f + 1); }, "FOG");
//            debugMenu->addButton("Color", nullptr, "FOG");
//            debugMenu->addSliderButton("Red", [=](float value) { const Color& fogColor = renderer.getFogColor();
//            renderer.setFogColor(Color(value, fogColor.g(), fogColor.b())); }, "FOG-Color");
//            debugMenu->addSliderButton("Green", [=](float value) {const Color& fogColor = renderer.getFogColor();
//            renderer.setFogColor(Color(fogColor.r(), value, fogColor.b()));  }, "FOG-Color");
//            debugMenu->addSliderButton("Blue", [=](float value) {const Color& fogColor = renderer.getFogColor();
//            renderer.setFogColor(Color(fogColor.r(), fogColor.g(), value)); }, "FOG-Color");
//        }
//
//    }
//}