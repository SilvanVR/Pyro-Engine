#include "post_process.h"

#include "vulkan-core/sub_renderer/post_processing_renderer/post_processing_renderer.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    PostProcess::PostProcess(const std::string& shaderName, const std::string& filePath, float resolutionModifier)
        : SinglePostProcessStep(PostProcessingRenderer::getRenderpass(), shaderName, filePath, resolutionModifier)
    {
        PostProcessingRenderer::addPostProcess(this);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    PostProcess::~PostProcess()
    {
        PostProcessingRenderer::removePostProcess(this);
    }


}