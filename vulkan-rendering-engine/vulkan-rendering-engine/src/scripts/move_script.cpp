#include "move_script.h"

#include "time/time.h"


CMove::CMove(Vec3f _axis, float _speed)
    : axis(_axis)
{
    this->speed = _speed;
}

void CMove::addedToNode(Pyro::Node * node)
{
    startPosition = parentNode->getTransform().position;
}

void CMove::update(float delta)
{
    Pyro::Transform& trans = parentNode->getTransform();

    trans.position = startPosition + (float)sin(Pyro::Time::getTotalRunningTimeInSeconds() * speed) * axis;
}
