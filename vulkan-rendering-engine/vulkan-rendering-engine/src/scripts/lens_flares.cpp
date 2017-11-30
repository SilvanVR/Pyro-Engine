#include "lens_flares.h"

#include "vulkan-core/sub_renderer/post_processing_renderer/post_processing_renderer.h"
#include "vulkan-core/vkTools/vk_tools.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    CLensFlares::CLensFlares(const std::vector<FlareInfo>& lensFlares, float spacing, float invisSpeedFactor)
        : Component()
    {
        // Notify the post-processing system for a new lens-flare component
        pLensFlares = PostProcessingRenderer::getLensFlareRenderer()->addNewLensFlares(lensFlares, spacing, invisSpeedFactor);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    CLensFlares::~CLensFlares()
    {
        // Remove lens-flares from post-processing system
        PostProcessingRenderer::getLensFlareRenderer()->removeLensFlares(pLensFlares);
    }

    //---------------------------------------------------------------------------
    //  Update
    //---------------------------------------------------------------------------

    void CLensFlares::update(float delta)
    {
        // Calculate screen-pos of attached object and send it to the post-processing stage
        const Mat4f& viewProjection = RenderingEngine::getCamera()->getViewProjection();
        Vec2f screenPosition = vkTools::worldToScreenPos(parentNode->getWorldPosition(), viewProjection);
        pLensFlares->setScreenPosition(screenPosition);
    }

    //---------------------------------------------------------------------------
    //  CDefaultLensFlares - Constructor
    //---------------------------------------------------------------------------

    CDefaultLensFlares::CDefaultLensFlares(float spacing, float invisSpeedFactor)
        : CLensFlares({
            { "/textures/lens_flares/tex4.png", 0.1f },
            { "/textures/lens_flares/tex2.png", 0.04f },
            { "/textures/lens_flares/tex5.png", 0.03f },
            { "/textures/lens_flares/tex7.png", 0.07f },
            { "/textures/lens_flares/tex3.png", 0.03f },
            { "/textures/lens_flares/tex5.png", 0.1f },
            { "/textures/lens_flares/tex4.png", 0.2f },
            { "/textures/lens_flares/tex8.png", 0.3f },
          }, spacing, invisSpeedFactor)
    {}

}