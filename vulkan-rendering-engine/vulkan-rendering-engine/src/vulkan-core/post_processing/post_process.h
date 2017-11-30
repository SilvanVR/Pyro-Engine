#ifndef POST_PROCESS_H_
#define POST_PROCESS_H_

#include "post_processing.h"

// Interface for creating own post-process shader.
// Name MUST be unique. (error message if not)
// Example Usage: 
// PostProcess testProcess("Test", "/shaders/post_process/test");
//
// For more advanced techniques (sampling not only from the last framebuffer)
// a lot more work is required (maybe i do a simpler version for that in the future)


namespace Pyro
{


    //---------------------------------------------------------------------------
    //  PostProcess class
    //---------------------------------------------------------------------------

    class PostProcess : public SinglePostProcessStep
    {
    public:
        PostProcess(const std::string& name, const std::string& filePath, float resolutionModifier = 1.0f);
        virtual ~PostProcess();
    };





}



#endif // !POST_PROCESS_H_

