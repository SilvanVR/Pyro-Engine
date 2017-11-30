#include "graphics_pipeline.h"

#include "vulkan-core/pipelines/renderpass/renderpass.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "vulkan-core/vulkan_base.h"
#include "shaders/shader.h"

#include <assert.h>

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    // Set default-values for all structs, needed for a VkPipeline. This Constructor is private. Creation of Pipelines through static-methods.
    GraphicsPipeline::GraphicsPipeline(VkDevice _device, Shader* shaders, Renderpass* _renderpass, 
                                       PipelineType type, bool _isParentPipe, GraphicsPipeline* parentPipeline)
        : device(_device), usedShaders(shaders), pipelineType(type), renderpass(_renderpass),
          isParentPipe(_isParentPipe), parentPipe(parentPipeline), pipelineCache(VK_NULL_HANDLE)
    {
        /* Setup Shaders */
        setupShaders();

        /* Input assembly state */
        setupInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        /* Viewport */
        setupViewport(nullptr, nullptr);

        /* Rasterizer */
        setupRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, VK_FALSE);

        /* Multisampler */
        setupMultisample(VK_FALSE);

        /* Depth and stencil */
        setupDepthStencil(VK_FALSE, VK_FALSE);

        /* Blending */
        setupColorBlend(VK_FALSE);

        /* Dynamic states */
        setupDynamicStates({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
    }

    //---------------------------------------------------------------------------
    //  Public Functions
    //---------------------------------------------------------------------------

    // Bind this pipeline to the given Command Buffer
    void GraphicsPipeline::bind(VkCommandBuffer cmd)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    // Recreate this pipeline from the given renderpass
    void GraphicsPipeline::recreate(Renderpass* renderpass)
    {
        /* Destroy old one */
        vkDestroyPipeline(device, pipeline, nullptr);

        /* Create new one */
        createVkGraphicsPipeline(renderpass);
    }

    //---------------------------------------------------------------------------
    //  Static Creation of GraphicPipelines
    //---------------------------------------------------------------------------

    GraphicsPipeline* GraphicsPipeline::createNewPipeline(Shader* shaders, PipelineType pipelineType, Renderpass* renderpass)
    {
        if (renderpass == nullptr)
            renderpass = VulkanBase::getRenderpass();

        switch (pipelineType)
        {
        case PipelineType::Basic:
            return GraphicsPipeline::createBasicPipeline(VulkanBase::getDevice(), shaders, renderpass);
        case PipelineType::Cubemap:
            return GraphicsPipeline::createCubemapPipeline(VulkanBase::getDevice(), shaders, renderpass);
        case PipelineType::GUI:
            return GraphicsPipeline::createGUIPipeline(VulkanBase::getDevice(), shaders, renderpass);
        case PipelineType::Shadowmap:
            return GraphicsPipeline::createShadowMapPipeline(VulkanBase::getDevice(), shaders, renderpass);
        case PipelineType::Wireframe:
            return GraphicsPipeline::createWireframePipeline(VulkanBase::getDevice(), shaders, renderpass);
        case PipelineType::AlphaBlend:
            return GraphicsPipeline::createAlphaBlendPipeline(VulkanBase::getDevice(), shaders, renderpass);
        case PipelineType::PostProcess:
            return GraphicsPipeline::createPostProcessPipeline(VulkanBase::getDevice(), shaders, renderpass);
        case PipelineType::Sun:
            return GraphicsPipeline::createSunPipeline(VulkanBase::getDevice(), shaders, renderpass);
        case PipelineType::Deferred:
            return GraphicsPipeline::createDeferredPipeline(VulkanBase::getDevice(), shaders, renderpass);
        case PipelineType::Light:
            return GraphicsPipeline::createLightPipeline(VulkanBase::getDevice(), shaders, renderpass);
        case PipelineType::DirLightStencil:
            return GraphicsPipeline::createDirLightStencilPipeline(VulkanBase::getDevice(), shaders, renderpass);
        case PipelineType::FOG:
            return GraphicsPipeline::createFOGPipeline(VulkanBase::getDevice(), shaders, renderpass);
        default:
            Logger::Log("GraphicsPipeline::createNewPipeline(): Given pipelineType is unknown!", LOGTYPE_ERROR);
        }
        return nullptr;
    }

    // Create a basic graphics pipeline from a given set of shaders, a renderpass and a vertex description
    GraphicsPipeline* GraphicsPipeline::createBasicPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
        bool isParentPipe, GraphicsPipeline* parentPipeline)
    {
        /* Create a new pipeline */
        GraphicsPipeline* pipe = new GraphicsPipeline(device, shaders, renderpass, PipelineType::Basic, isParentPipe, parentPipeline);

        /* Rasterizer */
        //pipe->setupRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, VK_FALSE);

        /* Depth and stencil */
        pipe->setupDepthStencil(VK_TRUE, VK_TRUE);
        
        /* Create VkPipeline */
        pipe->createVkGraphicsPipeline(renderpass);

        return pipe;
    }

    // Create a basic graphics pipeline from a given set of shaders, a renderpass and a vertex description
    GraphicsPipeline* GraphicsPipeline::createAlphaBlendPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
        bool isParentPipe, GraphicsPipeline* parentPipeline)
    {
        /* Create a new pipeline */
        GraphicsPipeline* pipe = new GraphicsPipeline(device, shaders, renderpass, PipelineType::AlphaBlend, isParentPipe, parentPipeline);

        /* Rasterizer */
        pipe->setupRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, VK_FALSE);

        /* Depth and stencil */
        pipe->setupDepthStencil(VK_TRUE, VK_TRUE);

        /* Blending */
        pipe->setupColorBlend(VK_TRUE);

        /* Create VkPipeline */
        pipe->createVkGraphicsPipeline(renderpass);

        return pipe;
    }

    // Create a wireframe pipeline
    GraphicsPipeline* GraphicsPipeline::createWireframePipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
        bool isParentPipe, GraphicsPipeline* parentPipeline)
    {
        /* Create a new pipeline */
        GraphicsPipeline* pipe = new GraphicsPipeline(device, shaders, renderpass, PipelineType::Wireframe, isParentPipe, parentPipeline);

        /* Rasterizer */
        pipe->setupRasterizer(VK_POLYGON_MODE_LINE, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);

        /* Depth and stencil */
        pipe->setupDepthStencil(VK_TRUE, VK_TRUE);
        
        /* Create VkPipeline */
        pipe->createVkGraphicsPipeline(renderpass);

        return pipe;
    }

    // Create a basic graphics pipeline from a given set of shaders, a renderpass and a vertex description
    GraphicsPipeline* GraphicsPipeline::createCubemapPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                              bool isParentPipe, GraphicsPipeline* parentPipeline)
    {
        /* Create a new pipeline */
        GraphicsPipeline* pipe = new GraphicsPipeline(device, shaders, renderpass, PipelineType::Cubemap, isParentPipe, parentPipeline);

        /* Rasterizer */
        pipe->setupRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE);

        /* Depth and stencil */
        pipe->setupDepthStencil(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

        /* Create VkPipeline */
        pipe->createVkGraphicsPipeline(renderpass);

        return pipe;
    }

    // Create a sun graphics pipeline from a given set of shaders, a renderpass and a vertex description
    GraphicsPipeline* GraphicsPipeline::createSunPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                          bool isParentPipe, GraphicsPipeline* parentPipeline)
    {
        /* Create a new pipeline */
        GraphicsPipeline* pipe = new GraphicsPipeline(device, shaders, renderpass, PipelineType::Sun, isParentPipe, parentPipeline);

        /* Rasterizer */
        pipe->setupRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);

        /* Blending */
        pipe->setupColorBlend(VK_TRUE);

        /* Depth and stencil */
        pipe->setupDepthStencil(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

        /* Create VkPipeline */
        pipe->createVkGraphicsPipeline(renderpass);

        return pipe;
    }

    // Create a blending (for text overlay) graphics pipeline from a given set of shaders, a renderpass and a vertex description
    GraphicsPipeline* GraphicsPipeline::createGUIPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
        bool isParentPipe, GraphicsPipeline* parentPipeline)
    {
        /* Create a new pipeline */
        GraphicsPipeline* pipe = new GraphicsPipeline(device, shaders, renderpass, PipelineType::GUI, isParentPipe, parentPipeline);

        /* Input assembly state */
        pipe->setupInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

        /* Rasterizer */
        pipe->setupRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);

        /* Blending */
        pipe->setupColorBlend(VK_TRUE);

        /* Create VkPipeline */
        pipe->createVkGraphicsPipeline(renderpass);

        return pipe;
    }

    // Create a shadow-map graphics pipeline from a given set of shaders, a renderpass and a vertex description
    GraphicsPipeline* GraphicsPipeline::createShadowMapPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
        bool isParentPipe, GraphicsPipeline* parentPipeline)
    {
        /* Create a new pipeline */
        GraphicsPipeline* pipe = new GraphicsPipeline(device, shaders, renderpass, PipelineType::Shadowmap, isParentPipe, parentPipeline);

        /* Rasterizer */
        pipe->setupRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, VK_FALSE);

        /* Depth and stencil */
        pipe->setupDepthStencil(VK_TRUE, VK_TRUE);

        /* Create VkPipeline */
        pipe->createVkGraphicsPipeline(renderpass);

        return pipe;
    }

    // Create a postprocess pipeline
    GraphicsPipeline* GraphicsPipeline::createPostProcessPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                                  bool isParentPipe, GraphicsPipeline* parentPipeline)
    {
        /* Create a new pipeline */
        GraphicsPipeline* pipe = new GraphicsPipeline(device, shaders, renderpass, PipelineType::PostProcess, isParentPipe, parentPipeline);

        /* Input assembly state */
        pipe->setupInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

        /* Rasterizer */
        pipe->setupRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);

        /* Depth and stencil */
        pipe->setupDepthStencil(VK_FALSE, VK_FALSE);

        /* Create VkPipeline */
        pipe->createVkGraphicsPipeline(renderpass);

        return pipe;
    }

    // Create a deferred pipeline
    GraphicsPipeline* GraphicsPipeline::createDeferredPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                               bool isParentPipe, GraphicsPipeline* parentPipeline)
    {
        /* Create a new pipeline */
        GraphicsPipeline* pipe = new GraphicsPipeline(device, shaders, renderpass, PipelineType::Deferred, isParentPipe, parentPipeline);

        /* Rasterizer */
        pipe->setupRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);

        /* Create VkPipeline */
        pipe->createVkGraphicsPipeline(renderpass);

        return pipe;
    }

    // Create a deferred pipeline
    GraphicsPipeline* GraphicsPipeline::createLightPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                            bool isParentPipe, GraphicsPipeline* parentPipeline)
    {
        /* Create a new pipeline */
        GraphicsPipeline* pipe = new GraphicsPipeline(device, shaders, renderpass, PipelineType::Light, isParentPipe, parentPipeline);

        /* Rasterizer */
        pipe->setupRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);

        /* Blending */
        pipe->setupColorBlend(VK_TRUE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD);

        /* Depth and stencil */
        pipe->setupDepthStencil(VK_FALSE, VK_FALSE);

        /* Create VkPipeline */
        pipe->createVkGraphicsPipeline(renderpass);

        return pipe;
    }

    // Create a pipeline !!! NOT USED ANYMORE !!!
    GraphicsPipeline* GraphicsPipeline::createDirLightStencilPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                                      bool isParentPipe, GraphicsPipeline* parentPipeline)
    {
        /* Create a new pipeline */
        GraphicsPipeline* pipe = new GraphicsPipeline(device, shaders, renderpass, PipelineType::DirLightStencil, isParentPipe, parentPipeline);

        /* Rasterizer */
        pipe->setupRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);

        /* Blending */
        pipe->setupColorBlend(VK_TRUE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD);

        VkStencilOpState front, back;
        front.failOp = VK_STENCIL_OP_KEEP;
        front.passOp = VK_STENCIL_OP_KEEP;
        front.depthFailOp = VK_STENCIL_OP_DECREMENT_AND_WRAP;
        front.compareOp = VK_COMPARE_OP_ALWAYS;
        front.compareMask = 255;
        front.writeMask = 255;
        front.reference = 0;

        back = front;
        back.depthFailOp = VK_STENCIL_OP_INCREMENT_AND_WRAP;

        /* Depth and stencil */
        pipe->setupDepthStencil(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_TRUE, front, back);

        /* Create VkPipeline */
        pipe->createVkGraphicsPipeline(renderpass);

        return pipe;
    }

    // Create a deferred pipeline
    GraphicsPipeline* GraphicsPipeline::createFOGPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                          bool isParentPipe, GraphicsPipeline* parentPipeline)
    {
        /* Create a new pipeline */
        GraphicsPipeline* pipe = new GraphicsPipeline(device, shaders, renderpass, PipelineType::FOG, isParentPipe, parentPipeline);

        /* Rasterizer */
        pipe->setupRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);

        /* Blending */
        pipe->setupColorBlend(VK_TRUE);

        /* Create VkPipeline */
        pipe->createVkGraphicsPipeline(renderpass);

        return pipe;
    }

    //---------------------------------------------------------------------------
    //  Private Functions
    //---------------------------------------------------------------------------

    // Create the VkGraphicsPipeline. Call this function when all structs are set.
    void GraphicsPipeline::createVkGraphicsPipeline(Renderpass* renderpass)
    {
        VkPipelineCreateFlags flags = 0;
        flags |= isParentPipe ? VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT : 0;
        flags |= parentPipe == nullptr ? 0 : VK_PIPELINE_CREATE_DERIVATIVE_BIT;

        /* Graphics pipeline */
        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
        graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineCreateInfo.pNext = nullptr;
        graphicsPipelineCreateInfo.flags = flags;
        graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        graphicsPipelineCreateInfo.pStages = shaderStages.data();
        graphicsPipelineCreateInfo.pVertexInputState = &vertexLayout.get();
        graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
        graphicsPipelineCreateInfo.pTessellationState = nullptr;
        graphicsPipelineCreateInfo.pViewportState = &viewportState;
        graphicsPipelineCreateInfo.pRasterizationState = &rasterizationState;
        graphicsPipelineCreateInfo.pMultisampleState = &multisampleState;
        graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilState;
        graphicsPipelineCreateInfo.pColorBlendState = &colorBlendState;
        graphicsPipelineCreateInfo.pDynamicState = &dynamicState;
        graphicsPipelineCreateInfo.layout = usedShaders->getPipelineLayout()->get();
        graphicsPipelineCreateInfo.renderPass = renderpass->get();
        graphicsPipelineCreateInfo.subpass = 0;
        graphicsPipelineCreateInfo.basePipelineHandle = parentPipe == nullptr ? VK_NULL_HANDLE : parentPipe->pipeline;
        graphicsPipelineCreateInfo.basePipelineIndex = -1;

        // Create Pipeline-Cache the first-time
        if (pipelineCache == VK_NULL_HANDLE)
        {
            VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
            pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
            pipelineCacheCreateInfo.initialDataSize = 0;
            vkCreatePipelineCache(device, &pipelineCacheCreateInfo, NULL, &pipelineCache);
        }

        VkResult res = vkCreateGraphicsPipelines(device, pipelineCache, 1, &graphicsPipelineCreateInfo, NULL, &pipeline);
        assert(res == VK_SUCCESS);
    }

    // Initialize the shaderStage-vector
    void GraphicsPipeline::setupShaders()
    {
        // Get Shader-Modules from the Shader-Class
        std::vector<ShaderModule*>& shaders = usedShaders->getShaderModules();

        // Set num shaders
        numShaders = static_cast<uint32_t>(shaders.size());

        // Resize std::vector
        shaderStages.resize(numShaders);

        for (uint32_t i = 0; i < numShaders; i++)
        {
            switch (shaders[i]->getShaderStage())
            {
            case ShaderStage::Vertex:
                // Get the vertex-layout from the Vertex-ShaderModule
                vertexLayout = shaders[i]->getVertexLayout();

                shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStages[i].pNext = nullptr;
                shaderStages[i].flags = 0;
                shaderStages[i].stage = VK_SHADER_STAGE_VERTEX_BIT;
                shaderStages[i].module = shaders[i]->getModule();
                shaderStages[i].pName = "main";
                shaderStages[i].pSpecializationInfo = nullptr;
                break;

            case ShaderStage::Fragment:
                fragmentEnabled = true;
                shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStages[i].pNext = nullptr;
                shaderStages[i].flags = 0;
                shaderStages[i].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                shaderStages[i].module = shaders[i]->getModule();
                shaderStages[i].pName = "main";
                shaderStages[i].pSpecializationInfo = nullptr;
                break;

            case ShaderStage::Geometry:
                geometryEnabled = true;
                shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStages[i].pNext = nullptr;
                shaderStages[i].flags = 0;
                shaderStages[i].stage = VK_SHADER_STAGE_GEOMETRY_BIT;
                shaderStages[i].module = shaders[i]->getModule();
                shaderStages[i].pName = "main";
                shaderStages[i].pSpecializationInfo = nullptr;
                break;

            case ShaderStage::Tessellation:
                tessellationEnabled = true;
                Logger::Log("Tessellation Shader not yet supported.", LOGTYPE_ERROR);
                break;

            default:
                Logger::Log("Unknown Shadertype in 'createBasicPipeline' detected.", LOGTYPE_ERROR);
            }
        }
    }


    // Helper functions for fast creation of single pipeline states
    // If you need more parameters, just overload the appropriate function

    void GraphicsPipeline::setupInputAssembly(const VkPrimitiveTopology& topology)
    {
        inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyState.pNext = nullptr;
        inputAssemblyState.flags = 0;
        inputAssemblyState.topology = topology;
        inputAssemblyState.primitiveRestartEnable = VK_FALSE;
    }

    void GraphicsPipeline::setupViewport(const VkViewport* viewport, const VkRect2D* scissor)
    {
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.pNext = nullptr;
        viewportState.viewportCount = 1;
        viewportState.pViewports = viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = scissor;
    }

    void GraphicsPipeline::setupRasterizer(const VkPolygonMode& polygonMode, const VkCullModeFlags& cullMode, const VkFrontFace& frontFace,
                                           const VkBool32& depthClampEnabled, const VkBool32& depthBiasEnabled)
    {
        setupRasterizer(polygonMode, cullMode, frontFace, depthClampEnabled, depthBiasEnabled, VK_FALSE);
    }

    void GraphicsPipeline::setupRasterizer(const VkPolygonMode& polygonMode, const VkCullModeFlags& cullMode, const VkFrontFace& frontFace, 
                                           const VkBool32& depthClampEnabled, const VkBool32& depthBiasEnabled, const VkBool32& rasterizerDiscardEnable)
    {
        rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationState.pNext = nullptr;
        rasterizationState.flags = 0;
        rasterizationState.depthClampEnable = depthClampEnabled;
        rasterizationState.rasterizerDiscardEnable = rasterizerDiscardEnable;
        rasterizationState.polygonMode = polygonMode;
        rasterizationState.cullMode = cullMode;
        rasterizationState.frontFace = frontFace;
        rasterizationState.depthBiasEnable = depthBiasEnabled;
        rasterizationState.depthBiasConstantFactor = 0;
        rasterizationState.depthBiasClamp = 0;
        rasterizationState.depthBiasSlopeFactor = 0;
        rasterizationState.lineWidth = 1;
    }

    void GraphicsPipeline::setupMultisample(const VkBool32& alphaToCoverageEnable)
    {
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.pNext = nullptr;
        multisampleState.flags = 0;
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleState.sampleShadingEnable = VK_FALSE;
        multisampleState.minSampleShading = 1.0f;
        multisampleState.pSampleMask = nullptr;
        multisampleState.alphaToCoverageEnable = alphaToCoverageEnable;
        multisampleState.alphaToOneEnable = VK_FALSE;
    }

    void GraphicsPipeline::setupDepthStencil(const VkBool32& depthTestEnable, const VkBool32& depthWriteEnable)
    {
        setupDepthStencil(depthTestEnable, depthWriteEnable, VK_COMPARE_OP_LESS_OR_EQUAL);
    }

    void GraphicsPipeline::setupDepthStencil(const VkBool32& depthTestEnable, const VkBool32& depthWriteEnable, const VkCompareOp& depthCompareOp)
    {
        setupDepthStencil(depthTestEnable, depthWriteEnable, depthCompareOp, VK_FALSE, {}, {});
    }

    void GraphicsPipeline::setupDepthStencil(const VkBool32& depthTestEnable, const VkBool32& depthWriteEnable, const VkCompareOp& depthCompareOp,
                                             const VkBool32& stencilTestEnable, const VkStencilOpState& front, const VkStencilOpState& back)
    {
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilState.pNext = nullptr;
        depthStencilState.flags = 0;
        depthStencilState.depthTestEnable = depthTestEnable;
        depthStencilState.depthWriteEnable = depthWriteEnable;
        depthStencilState.depthCompareOp = depthCompareOp;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.stencilTestEnable = stencilTestEnable;
        depthStencilState.back = back;
        depthStencilState.front = front;
        depthStencilState.minDepthBounds = 0.0f;
        depthStencilState.maxDepthBounds = 1.0f;
    }

    void GraphicsPipeline::setupColorBlend(VkBool32 blendingEnable)
    {
        setupColorBlend(blendingEnable, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD);
    }

    void GraphicsPipeline::setupColorBlend(VkBool32 blendingEnable, VkBlendFactor srcFac, VkBlendFactor dstFac, VkBlendOp blendOp)
    {
        // Clear previous blend-attachment-states
        colorBlendAttachmentStates.clear();

        auto colorAttachments = renderpass->getColorAttachments();

        for (uint32_t i = 0; i < colorAttachments.size(); i++)
        {
            VkPipelineColorBlendAttachmentState colorBlend = {};
            colorBlend.blendEnable          = blendingEnable;
            colorBlend.srcColorBlendFactor  = srcFac;
            colorBlend.dstColorBlendFactor  = dstFac;
            colorBlend.colorBlendOp         = blendOp;
            colorBlend.srcAlphaBlendFactor  = VK_BLEND_FACTOR_ZERO;
            colorBlend.dstAlphaBlendFactor  = VK_BLEND_FACTOR_ONE;
            colorBlend.alphaBlendOp         = VK_BLEND_OP_ADD;
            colorBlend.colorWriteMask       = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

            colorBlendAttachmentStates.push_back(colorBlend);
        }

        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.pNext = nullptr;
        colorBlendState.flags = 0;
        colorBlendState.logicOpEnable = VK_FALSE;
        colorBlendState.logicOp = VK_LOGIC_OP_COPY;
        colorBlendState.attachmentCount = static_cast<uint32_t>(colorBlendAttachmentStates.size());
        colorBlendState.pAttachments = colorBlendAttachmentStates.data();
        colorBlendState.blendConstants[0] = 0;
        colorBlendState.blendConstants[1] = 0;
        colorBlendState.blendConstants[2] = 0;
        colorBlendState.blendConstants[3] = 0;
    }

    void GraphicsPipeline::setupDynamicStates(std::vector<VkDynamicState> dynamicStateEnables)
    {
        dynamicStates = dynamicStateEnables;

        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pNext = nullptr;
        dynamicState.flags = 0;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();
    }

}