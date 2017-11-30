#include "light_spawn.h"


#include "Input/input.h"
#include "move_script.h"
#include "camera_mov.h"

using namespace Pyro;

void CLightSpawn::update(float delta)
{

    if (Input::getKeyDownOnce(keyCode))
    {
        Color randomColor(Random::randomFloat(), Random::randomFloat(), Random::randomFloat());
        Point3f spawnPosition = parentNode->getWorldPosition();
        float intensity = 20.0f;
        Vec3f attenuation = Vec3f(.0f, .0f, 1.0f);

        auto pointLight = std::make_unique<PointLight>(randomColor, intensity, attenuation, spawnPosition, nullptr);
        pointLight->addComponent(new CMove(Vec3f(20,0,0), 2));
        pointLights.push_back(std::move(pointLight));
    }

    //if (Input::getKeyDownOnce(KeyCodes::J))
    //{
    //    if (pointLights.size() > 0)
    //        pointLights.pop_back();
    //}

}
