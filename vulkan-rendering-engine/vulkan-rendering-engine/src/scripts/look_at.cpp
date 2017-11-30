#include "look_at.h"


LookAt::LookAt(Pyro::Node* _target)
    : target(_target)
{ 
}

void LookAt::update(float delta)
{
    parentNode->getTransform().lookAt(target->getWorldPosition());
}