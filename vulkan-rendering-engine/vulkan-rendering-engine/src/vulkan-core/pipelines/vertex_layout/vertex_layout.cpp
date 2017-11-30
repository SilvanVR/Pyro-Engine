#include "vertex_layout.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------


    VertexLayout::VertexLayout(const std::vector<Layout>& vertexLayout)
    {
        // Attribute descriptions
        attributeDescriptions.resize(vertexLayout.size());

        uint32_t offset = 0;
        for (unsigned int i = 0; i < vertexLayout.size(); i++)
        {
            switch (vertexLayout[i])
            {
            case Layout::VEC3F:
                attributeDescriptions[i].binding = VERTEX_BUFFER_BIND_ID;
                attributeDescriptions[i].location = i;
                attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[i].offset = offset;
                offset += 3 * sizeof(float);
                break;
            case Layout::VEC2F:
                attributeDescriptions[i].binding = VERTEX_BUFFER_BIND_ID;
                attributeDescriptions[i].location = i;
                attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[i].offset = offset;
                offset += 2 * sizeof(float);
                break;
            }
        }

        // Binding description
        bindingDescriptions.resize(vertexLayout.size() == 0 ? 0 : 1);
        for (unsigned int i = 0; i < bindingDescriptions.size(); i++)
        {
            bindingDescriptions[i].binding = i;
            bindingDescriptions[i].stride = offset;
            bindingDescriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }

        // Assign to pipeline vertex description
        pipelineVertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipelineVertexInput.pNext = nullptr;
        pipelineVertexInput.flags = 0;
        pipelineVertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        pipelineVertexInput.pVertexBindingDescriptions = bindingDescriptions.data();
        pipelineVertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        pipelineVertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();
    }

    //---------------------------------------------------------------------------
    //  Operator Overloading
    //---------------------------------------------------------------------------

    // Overload "=" operator, because VkPipelineVertexInputStateCreateInfo stores pointers to "bindingDescriptions" and "attributeDescriptions"
    VertexLayout& VertexLayout::operator=(const VertexLayout& vertexLayout)
    {
        bindingDescriptions = vertexLayout.bindingDescriptions;
        attributeDescriptions = vertexLayout.attributeDescriptions;

        // Assign to pipeline vertex description
        pipelineVertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipelineVertexInput.pNext = nullptr;
        pipelineVertexInput.flags = 0;
        pipelineVertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        pipelineVertexInput.pVertexBindingDescriptions = bindingDescriptions.data();
        pipelineVertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        pipelineVertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();

        return *this;
    }

    VertexLayout::VertexLayout(const VertexLayout& vertexLayout)
    {
        *this = vertexLayout;
    }

}