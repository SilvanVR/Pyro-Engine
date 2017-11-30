/*
*  GraphicsPipeline-Class header file.
*  Encapsulates a VkPipeline.
*  Create new pipelines per static methods.
*
*  Date:    25.04.2016
*  Creator: Silvan Hau
*/

#ifndef PIPELINE_H_
#define PIPELINE_H_

#include "vertex_layout/vertex_layout.h"
#include "build_options.h"

namespace Pyro
{

    enum class PipelineType
    {
        Basic,
        Wireframe,
        AlphaBlend,
        Cubemap,
        GUI,
        Shadowmap,
        PostProcess,
        Sun,
        Deferred,
        Light,
        DirLightStencil,
        FOG
    };

    //---------------------------------------------------------------------------
    //  GraphicsPipeline class
    //---------------------------------------------------------------------------

    class Shader;
    class Renderpass;

    class GraphicsPipeline
    {
    public:
        ~GraphicsPipeline() { vkDestroyPipelineCache(device, pipelineCache, nullptr); vkDestroyPipeline(device, pipeline, nullptr); };

        // Return the type of this pipeline
        const PipelineType& getPipelineType() const { return pipelineType; }

        // Recreate this pipeline from the given renderpass
        void recreate(Renderpass* renderpass);

        // Bind this pipeline to the given Command Buffer.
        void bind(VkCommandBuffer cmd);

    private:
        // Set default-values for all structs, needed for a VkPipeline. This Constructor is private. Creation of this class through static-methods.
        GraphicsPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass, PipelineType type, bool isParentPipe, GraphicsPipeline* parentPipeline);

        // forbid copy and copy assignment
        GraphicsPipeline(const GraphicsPipeline& pipe);
        GraphicsPipeline& operator=(const GraphicsPipeline& pipe) {};

        // Need a reference to the logical device for destruction 
        VkDevice device;

        // Handle to the VkPipeline
        VkPipeline pipeline;

        // The type of this pipeline
        PipelineType pipelineType;

        // Pipeline-Cache for faster recreation
        VkPipelineCache pipelineCache;

        // Handle to parent-pipeline if this is a derivate
        GraphicsPipeline* parentPipe = nullptr;

        // The renderpass this pipeline was created with
        Renderpass* renderpass;

        // True if this is pipeline was created with the "VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT" - flag
        bool        isParentPipe = false;

        // Some information about the pipeline
        Shader*     usedShaders = nullptr;
        uint32_t    numShaders = 0;
        bool        fragmentEnabled = false;
        bool        geometryEnabled = false;
        bool        tessellationEnabled = false;

        // Create the VkGraphicsPipeline. Call this function when all structs are set.
        void createVkGraphicsPipeline(Renderpass* renderpass);

        // Initialize the shaderStage-vector
        void setupShaders();

        // Store every state for recreate the VkPipeline (currently when window size changes)
        VkPipelineInputAssemblyStateCreateInfo       inputAssemblyState = {};
        VkPipelineViewportStateCreateInfo            viewportState = {};
        VkPipelineRasterizationStateCreateInfo       rasterizationState = {};
        VkPipelineMultisampleStateCreateInfo         multisampleState = {};
        VkPipelineDepthStencilStateCreateInfo        depthStencilState = {};
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
        VkPipelineColorBlendStateCreateInfo          colorBlendState = {};
        std::vector<VkDynamicState>                  dynamicStates;
        VkPipelineDynamicStateCreateInfo             dynamicState = {};
        VertexLayout                                 vertexLayout;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        // Helper functions for fast creation of single pipeline states
        // If you need more parameters, just overload the function you need
        void setupInputAssembly(const VkPrimitiveTopology& topology);
        void setupViewport(const VkViewport* viewport, const VkRect2D* scissor);
        void setupRasterizer(const VkPolygonMode& polygonMode, const VkCullModeFlags& cullMode, const VkFrontFace& frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                             const VkBool32& depthClampEnabled = VK_FALSE, const VkBool32& depthBiasEnabled = VK_FALSE);
        void setupRasterizer(const VkPolygonMode& polygonMode, const VkCullModeFlags& cullMode, const VkFrontFace& frontFace,
                             const VkBool32& depthClampEnabled, const VkBool32& depthBiasEnabled, const VkBool32& rasterizerDiscardEnable);
        void setupMultisample(const VkBool32& alphaToCoverageEnable);
        void setupDepthStencil(const VkBool32& depthTestEnable, const VkBool32& depthWriteEnable);
        void setupDepthStencil(const VkBool32& depthTestEnable, const VkBool32& depthWriteEnable, const VkCompareOp& depthCompareOp);
        void setupDepthStencil(const VkBool32& depthTestEnable, const VkBool32& depthWriteEnable, const VkCompareOp& depthCompareOp,
                               const VkBool32& stencilTestEnable, const VkStencilOpState& front, const VkStencilOpState& back);
        void setupColorBlend(VkBool32 blendingEnable = VK_FALSE);
        void setupColorBlend(VkBool32 blendingEnable, VkBlendFactor srcFac, VkBlendFactor dstFac, VkBlendOp blendOp);
        void setupDynamicStates(std::vector<VkDynamicState> dynamicStateEnables);

    public:
        // Create a new pipeline from the given shaders, pipeline-type and renderpass
        static GraphicsPipeline* createNewPipeline(Shader* shaders, PipelineType pipelineType, Renderpass* renderpass);

    private:
        // Create a basic graphics pipeline from a given set of shaders, a renderpass and a vertex description
        static GraphicsPipeline* createBasicPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                     bool isParentPipe = false, GraphicsPipeline* parentPipeline = nullptr);

        // Create a basic graphics pipeline from a given set of shaders, a renderpass and a vertex description
        static GraphicsPipeline* createAlphaBlendPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                          bool isParentPipe = false, GraphicsPipeline* parentPipeline = nullptr);

        // Create a wireframe pipeline
        static GraphicsPipeline* createWireframePipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                         bool isParentPipe = false, GraphicsPipeline* parentPipeline = nullptr);

        // Create a cubemap pipeline
        static GraphicsPipeline* createCubemapPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                       bool isParentPipe = false, GraphicsPipeline* parentPipeline = nullptr);

        // Create a sun pipeline
        static GraphicsPipeline* createSunPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                   bool isParentPipe = false, GraphicsPipeline* parentPipeline = nullptr);

        // Create a text overlay pipeline
        static GraphicsPipeline* createGUIPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                   bool isParentPipe = false, GraphicsPipeline* parentPipeline = nullptr);

        // Create a shadowmap pipeline
        static GraphicsPipeline* createShadowMapPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                         bool isParentPipe = false, GraphicsPipeline* parentPipeline = nullptr);

        // Create a postprocess pipeline
        static GraphicsPipeline* createPostProcessPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                           bool isParentPipe = false, GraphicsPipeline* parentPipeline = nullptr);

        // Create a deferred pipeline
        static GraphicsPipeline* createDeferredPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                       bool isParentPipe = false, GraphicsPipeline* parentPipeline = nullptr);

        // Create a pipeline for deferred-lighting
        static GraphicsPipeline* createLightPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                     bool isParentPipe = false, GraphicsPipeline* parentPipeline = nullptr);

        // Create a pipeline for deferred-lighting
        static GraphicsPipeline* createDirLightStencilPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                               bool isParentPipe = false, GraphicsPipeline* parentPipeline = nullptr);


        // Create a pipeline for fog
        static GraphicsPipeline* createFOGPipeline(VkDevice device, Shader* shaders, Renderpass* renderpass,
                                                   bool isParentPipe = false, GraphicsPipeline* parentPipeline = nullptr);
    };

}

#endif // !PIPELINE_H_
