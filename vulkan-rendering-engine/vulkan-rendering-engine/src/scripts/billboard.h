#ifndef BILLBOARD_H_
#define BILLBOARD_H_

#include "vulkan-core/script_interface.hpp"

class CBillboard : public Pyro::Component
{

public:
    CBillboard(Pyro::Node* target, const std::string& name, Pyro::TexturePtr tex);

    void update(float delta) override;
    void addedToNode(Pyro::Node* node) override;

    void setIsActive(bool b) override;

private:
    Pyro::Node* target;
    Pyro::Quad* billboard;
};

class CLightBillboard : public CBillboard
{
public:
    CLightBillboard();
};


#endif // !BILLBOARD_H_
