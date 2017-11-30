/*
*  VertexLayout-Class header file.
*  Encapsulates a VkPipelineVertexInputStateCreateInfo.
*
*  Date:    25.04.2016
*  Author:  Silvan Hau
*/

#ifndef VERTEX_LAYOUT_H_
#define VERTEX_LAYOUT_H_

#include "build_options.h"
#include <vector>

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  VertexLayout class
    //---------------------------------------------------------------------------

    class VertexLayout
    {
    public:
        enum Layout
        {
            VEC2F,
            VEC3F
        };

        VertexLayout() {};
        VertexLayout(const std::vector<Layout>& vertexLayout);
        ~VertexLayout() {};

        // Return the pipelineVertexInputState, which needs the pipeline at creation time
        VkPipelineVertexInputStateCreateInfo& get() { return pipelineVertexInput; }

        // Overload "=" operator and copy-constructor, because VkPipelineVertexInputStateCreateInfo stores pointers to "bindingDescriptions" and "attributeDescriptions"
        VertexLayout& operator=(const VertexLayout& vertexLayout);
        VertexLayout(const VertexLayout& vertexLayout);

    private:
        VkPipelineVertexInputStateCreateInfo pipelineVertexInput;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    };



}



#endif // !VERTEX_LAYOUT_H_
