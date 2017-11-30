#ifndef LENS_FLARES_H_
#define LENS_FLARES_H_

#include "../vulkan-core/script_interface.hpp"

// Example Code for different lens-flares 
//std::vector<FlareInfo> lensFlareInfos = {
//    { "/textures/lens_flares/tex4.png", 0.1f },
//    { "/textures/lens_flares/tex2.png", 0.04f },
//    { "/textures/lens_flares/tex5.png", 0.03f },
//    { "/textures/lens_flares/tex7.png", 0.07f },
//    { "/textures/lens_flares/tex3.png", 0.03f },
//    { "/textures/lens_flares/tex5.png", 0.1f },
//    { "/textures/lens_flares/tex4.png", 0.2f },
//    { "/textures/lens_flares/tex8.png", 0.3f },
//};

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  CLensFlares Class
    //---------------------------------------------------------------------------

    // Engine internal component
    class CLensFlares : public Component
    {
        LensFlares* pLensFlares; // Lens-Flare screen effect

    public:
        // Add a lens-flare component to a node.
        // Each time the camera looks at this object, the given textures 
        // will be rendered as small screen quads onto the screen.
        // @ lensFlares: Information about the size and the file-path of the flare-textures
        // @ spacing: The distance from the lens-flares to each other
        // @ invisSpeedFactor: How fast the lens-flares will become invisible
        CLensFlares(const std::vector<FlareInfo>& lensFlares, float spacing = 4.0f, float invisSpeedFactor = 1.0f);
        ~CLensFlares();

        void update(float delta) override;
    };

    //---------------------------------------------------------------------------
    //  CDefaultLensFlares Class
    //---------------------------------------------------------------------------

    class CDefaultLensFlares : public CLensFlares
    {
    public:
        // Add a lens-flare component to a node.
        // Each time the camera looks at this object, the given textures 
        // will be rendered as small screen quads onto the screen.
        // @ spacing: The distance from the lens-flares to each other
        // @ invisSpeedFactor: How fast the lens-flares will become invisible
        CDefaultLensFlares(float spacing = 4.0f, float invisSpeedFactor = 1.0f);
    };


}


#endif // !LENS_FLARES_H_
