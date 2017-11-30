#include "rotation_script.h"

CRotate::CRotate(float _rotateAmt)
    : rotateAmt(_rotateAmt)
{
}

void CRotate::update(float delta)
{
    Pyro::Transform& trans = parentNode->getTransform();

    trans.rotation *= Quatf(Vec3f::up, rotateAmt * delta);
}
