#ifndef ROTATION_SCRIPT_H_
#define ROTATION_SCRIPT_H_

#include "vulkan-core/script_interface.hpp"

class CRotate : public Pyro::Component
{
public:
    CRotate(float rotateAmt);

    void update(float delta) override;

private:
    float rotateAmt;
};


#endif // !ROTATION_SCRIPT_H_
