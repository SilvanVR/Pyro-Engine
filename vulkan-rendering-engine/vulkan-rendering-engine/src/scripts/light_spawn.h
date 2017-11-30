#ifndef LIGHT_SPAWN_H_
#define LIGHT_SPAWN_H_

#include "vulkan-core/script_interface.hpp"


class CLightSpawn : public Pyro::Component
{

public:
    CLightSpawn(int _keyCode) : keyCode(_keyCode) {}

    void update(float delta) override;

private:
    std::vector<std::unique_ptr<Pyro::PointLight>> pointLights;

    int keyCode;
};

#endif // !LIGHT_SPAWN_H_
