#ifndef OBJECT_SPAWN_H_
#define OBJECT_SPAWN_H_

#include "vulkan-core/script_interface.hpp"


class CObjectSpawn : public Pyro::Component
{

public:
    CObjectSpawn(int _keyCode);

    void update(float delta) override;

private:
    std::vector<Pyro::Renderable*> renderables;
    Pyro::TexturePtr texture;
    Pyro::PBRMaterialPtr material;

    int keyCode;
};

#endif // !OBJECT_SPAWN_H_
