#include "interaction_script.h"

#include "Input/input.h"

namespace Pyro
{

    CInteract::CInteract(float rayDistance)
        : maxRayDistance(rayDistance)
    {
        backgroundTexture = TEXTURE("/textures/button4.dds");

        // Background Texture
        backgroundImage = new GUIImage(backgroundTexture, Rectf(470, 110), Color(0.8f,0.8f,0.8f,0.8f), Anchor::CenterBottom);
        backgroundImage->setLocalPos(Vec2f(-backgroundImage->getWidth() / 2.0f, -backgroundImage->getHeight()));

        // NAME - TEXT
        FontPtr font = FONT_GET_DEFAULT();
        objectNameText = new GUIText("", Vec2f(0.0f, 10.0f + font->getTextHeight("Name:")), font,
            Color::BLACK, Vec2f(1.0f, 1.0f), Anchor::CenterTop, TextAlign::CENTER);
        objectNameText->setParent(backgroundImage);

        // POSITION - TEXT
        objectPositionText = new GUIText("", Vec2f(10.0f, 50.0f), font,
            Color::RED, Vec2f(1.0f, 1.0f), Anchor::TopLeft, TextAlign::LEFT);
        objectPositionText->setParent(backgroundImage);

        // ROTATION - TEXT
        objectRotationText = new GUIText("", Vec2f(0.0f, 15.0f), font,
            Color::VIOLET, Vec2f(1.0f, 1.0f), Anchor::CenterLeft, TextAlign::LEFT);
        objectRotationText->setParent(objectPositionText);

        // SCALE - TEXT
        objectScaleText = new GUIText("", Vec2f(0.0f, 15.0f), font,
            Color::BLUE, Vec2f(1.0f, 1.0f), Anchor::CenterLeft, TextAlign::LEFT);
        objectScaleText->setParent(objectRotationText);

        componentGUI.add({ backgroundImage, objectNameText, objectPositionText, objectRotationText, objectScaleText });
    }

    CInteract::~CInteract()
    {
    }

    void CInteract::update(float delta)
    {

        if (Input::getKeyDownOnce(KeyCodes::LMOUSEBUTTON))
        {
            if (selectedObject != nullptr)
            {
                selectedObject = nullptr;
            }
            else
            {
                const Ray& currentRay = MousePicker::getCurrentRay(maxRayDistance);
                HitInfo hitInfo = MousePicker::raycast(currentRay);

                if (hitInfo.node != nullptr)
                {
                    selectedObject = hitInfo.node;
                    currentDistance = (currentRay.getOrigin() - selectedObject->getWorldPosition()).magnitude();
                }

            }

        }
        else
        {

            if (selectedObject != nullptr)
            {
                const Ray& currentRay = MousePicker::getCurrentRay();

                Point3f hitPoint = currentRay.getOrigin() + currentRay.getDirection() * currentDistance;

                // Transform the hitPoint into the parents coordinate system
                // This is necessary if the object is a child of another object
                const Transform& parentTransform = selectedObject->getParent()->getWorldTransform();
                selectedObject->getTransform().position = parentTransform.getTransformationMatrix().inverse() * hitPoint;

                objectNameText->setText(selectedObject->getName());

                const Point3f& pos = selectedObject->getTransform().position;
                std::string positionString = "X: " + std::to_string(pos.x()) + " Y: " + std::to_string(pos.y()) + " Z: " + std::to_string(pos.z());
                objectPositionText->setText(positionString);

                const Vec3f& rot = selectedObject->getTransform().rotation.toEulerAngles();
                std::string rotationString = "X: " + std::to_string(rot.x()) + " Y: " + std::to_string(rot.y()) + " Z: " + std::to_string(rot.z());
                objectRotationText->setText(rotationString);

                const Vec3f& scale = selectedObject->getTransform().scale;
                std::string scaleString = "X: " + std::to_string(scale.x()) + " Y: " + std::to_string(scale.y()) + " Z: " + std::to_string(scale.z());
                objectScaleText->setText(scaleString);

                componentGUI.setIsActive(true);

                Vec3f& objectScale = selectedObject->getTransform().scale;
                float scaleAmt = delta;
                if (Input::getKeyDown(KeyCodes::KP_ADD))
                    selectedObject->getTransform().scale = Vec3f(objectScale.x() + scaleAmt, objectScale.y() + scaleAmt, objectScale.z() + scaleAmt);
                else if (Input::getKeyDown(KeyCodes::KP_SUBTRACT))
                    selectedObject->getTransform().scale = Vec3f(objectScale.x() - scaleAmt, objectScale.y() - scaleAmt, objectScale.z() - scaleAmt);

                if (Input::getKeyDownOnce(KeyCodes::DEL))
                {
                    delete selectedObject;
                    selectedObject = nullptr;
                }
            }
            else
            {
                componentGUI.setIsActive(false);
            }
        }

        // Modify Metallic / Roughness of selected object
      /*  if (selectedObject) {
            Renderable* selectedObj = dynamic_cast<Renderable*>(selectedObject);
            if (selectedObj)
            {
                PBRMaterial* m = dynamic_cast<PBRMaterial*>(selectedObj->getMaterial());

                float step = 0.25f;
                if (Input::getKeyDownOnce(KeyCodes::KP_8))
                {
                    float roughness = m->getMatRoughness() + step;
                    float newRoughness = Mathf::clamp(roughness, 0, 1);
                    m->setMatRoughness(newRoughness);
                }
                if (Input::getKeyDownOnce(KeyCodes::KP_2))
                {
                    float roughness = m->getMatRoughness() - step;
                    float newRoughness = Mathf::clamp(roughness, 0, 1);
                    m->setMatRoughness(newRoughness);
                }

                if (Input::getKeyDownOnce(KeyCodes::KP_4))
                {
                    float metallic = m->getMatMetallic() + step;
                    float newMetallic = Mathf::clamp(metallic, 0, 1);
                    m->setMatMetallic(newMetallic);
                }
                if (Input::getKeyDownOnce(KeyCodes::KP_6))
                {
                    float metallic = m->getMatMetallic() - step;
                    float newMetallic = Mathf::clamp(metallic, 0, 1);
                    m->setMatMetallic(newMetallic);
                }
            }
        }
*/
    }

} // !namespace pyro