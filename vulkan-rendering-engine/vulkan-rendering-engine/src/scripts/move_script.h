#ifndef MOVE_SCRIPT_H_
#define MOVE_SCRIPT_H_

#include "vulkan-core/script_interface.hpp"

class CMove : public Pyro::Component
{
public:
    CMove(Vec3f axis, float speed);

    void update(float delta) override;
    void addedToNode(Pyro::Node* node) override;

private:
    Vec3f axis;
    float speed;

    Point3f startPosition;
};



#endif // !MOVE_SCRIPT_H_

