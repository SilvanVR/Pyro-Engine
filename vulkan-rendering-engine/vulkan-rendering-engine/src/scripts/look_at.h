#ifndef LOOK_AT_H_
#define LOOK_AT_H_

#include "vulkan-core/script_interface.hpp"

class LookAt : public Pyro::Component
{

public:
    LookAt(Pyro::Node* target);

    void update(float delta) override;

private:
    Pyro::Node* target;
};


#endif // !LOOK_AT_H_
