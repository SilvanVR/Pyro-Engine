#include "billboard.h"

using namespace Pyro;

CBillboard::CBillboard(Pyro::Node* _target, const std::string& name, Pyro::TexturePtr tex)
    : target(_target)
{
    auto mat = MATERIAL(SHADER(SHADER_FW_BILLBOARD));
    mat->setTexture("BillboardTexture", tex);
    billboard = new Quad(name, mat);
    billboard->changeLayer({ LAYER_IGNORE_SHADOW_RENDERING });
}

void CBillboard::addedToNode(Node* node)
{
    billboard->getTransform().position = parentNode->getWorldPosition();
    if(!parentNode->isActive()) billboard->setIsActive(false);
}

void CBillboard::update(float delta)
{
    billboard->getTransform().lookAt(target->getWorldPosition());
    parentNode->getTransform().position = billboard->getWorldPosition();
}

void CBillboard::setIsActive(bool b)
{
    Component::setIsActive(b);
    billboard->setIsActive(b);
}

CLightBillboard::CLightBillboard()
    : CBillboard(RenderingEngine::getCamera(), "Light", TEXTURE({ "/textures/light_bulb.dds",
                 std::make_shared<Sampler>(1.0f, FILTER_NEAREST, FILTER_NEAREST, MIPMAP_MODE_NEAREST) }))
{
}