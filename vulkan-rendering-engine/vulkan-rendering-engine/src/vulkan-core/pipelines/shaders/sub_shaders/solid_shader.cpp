#include "solid_shader.h"

#include "vulkan-core/rendering_engine.h"

namespace ENGINE
{


    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Update uniforms in this shader
    void SolidShader::updateUniforms(RenderingEngine* renderer)
    {
        // Update camera position in vertex shader
        setVec3f("CameraPosVertex", renderer->getCamera()->getWorldPosition());
    }


}