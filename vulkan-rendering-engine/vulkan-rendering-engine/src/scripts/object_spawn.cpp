#include "object_spawn.h"

#include "Input/input.h"
#include "move_script.h"
#include "camera_mov.h"

using namespace Pyro;

CObjectSpawn::CObjectSpawn(int _keyCode) : keyCode(_keyCode)
{
    texture = TEXTURE("/textures/bricks.dds");
    material = PBRMATERIAL(texture);
}


void CObjectSpawn::update(float delta)
{

    if (Input::getKeyDownOnce(keyCode))
    {
        Point3f spawnPosition = parentNode->getWorldPosition();

        auto cube = new Cube(material, Transform(spawnPosition));
        renderables.push_back(cube);
    }

    //if (Input::getKeyDownOnce(KeyCodes::J))
    //{
    //    if (renderables.size() > 0)
    //    {
    //        delete renderables.front();
    //        renderables.erase(renderables.begin());
    //    }
    //}

}
