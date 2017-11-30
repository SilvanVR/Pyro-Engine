#include "post_processing.h"

#include "vulkan-core/sub_renderer/post_processing_renderer/post_processing_renderer.h"
#include "vulkan-core/scene_graph/nodes/renderables/renderable.h"
#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/pipelines/renderpass/renderpass.h"
#include "vulkan-core/util_classes/vulkan_other.h"
#include "vulkan-core/rendering_engine.h"

namespace Pyro
{

    static const Vec2f& centerOnScreen = Vec2f(0.5f, 0.5f);

    //---------------------------------------------------------------------------
    //  SinglePostProcessStep - Class
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    SinglePostProcessStep::SinglePostProcessStep(Renderpass* _renderpass, const std::string& shaderName, 
                                                 const std::string& filePath, float _resolutionModifier, 
                                                 PipelineType pipelineType, bool createFramebuffer)
        : renderpass(_renderpass), sampler(PostProcessingRenderer::getSampler()), resolutionModifier(_resolutionModifier)
    {
        // Create the shader
        ShaderParams params(shaderName, filePath, pipelineType, renderpass);
        shader = SHADER(params);

        // Create the framebuffer containing a color-image
        if (resolutionModifier > 0.0f && createFramebuffer)
        {
            uint32_t width  = static_cast<uint32_t>(VulkanBase::getFinalWidth() * resolutionModifier);
            uint32_t height = static_cast<uint32_t>(VulkanBase::getFinalHeight() * resolutionModifier);
            setupFramebuffer(width, height);
        }
    }

    // Constructor which create a framebuffer with the specified size
    SinglePostProcessStep::SinglePostProcessStep(Renderpass* _renderpass, const uint32_t& width, const uint32_t& height,
                                                 const std::string& shaderName, const std::string& filePath)
        : renderpass(_renderpass), sampler(PostProcessingRenderer::getSampler())
    {
        // Create the shader
        ShaderParams params(shaderName, filePath, PipelineType::PostProcess, renderpass);
        shader = SHADER(params);
        setupFramebuffer(width, height);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    SinglePostProcessStep::~SinglePostProcessStep()
    {
        if(framebuffer != nullptr)
            delete framebuffer;
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Record all commands into the given cmd which renders this post-process-step
    // Some stages might use the original scene-framebuffer, thats why it is passed separately too
    void SinglePostProcessStep::record(CommandBuffer* cmd, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer)
    {
        record(cmd, framebuffer, inputs);
    }

    // Record all commands into the given cmd which renders this post-process-step
    // in the target FBO using the inputs-FBO as inputs
    void SinglePostProcessStep::record(CommandBuffer* cmd, Framebuffer* targetFBO, const std::vector<Framebuffer*>& inputs)
    {
        std::vector<std::shared_ptr<VulkanImageView>> imageViews;
        for (uint32_t i = 0; i < inputs.size(); i++)
            imageViews.push_back(inputs[i]->getColorView());
        record(cmd, renderpass, targetFBO, imageViews);
    }

    // Record all commands into the given cmd which renders this post-process-step using the renderpass / framebuffer from this class
    void SinglePostProcessStep::record(CommandBuffer* cmd, const std::vector<std::shared_ptr<VulkanImageView>>& imageViews)
    {
        record(cmd, renderpass, framebuffer, imageViews);
    }

    // Record all commands into the given cmd which renders this post-process-step
    // in the target FBO using the inputs-FBO as inputs
    void SinglePostProcessStep::record(CommandBuffer* commandBuffer, Renderpass* renderpass, Framebuffer* targetFBO, 
                                       const std::vector<std::shared_ptr<VulkanImageView>>& imageViews)
    {
        VkCommandBuffer cmd = commandBuffer->get();

        // This ensures that we can sample from the previous-framebuffer only until all writes to it have been finished
        commandBuffer->pipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 
                                       VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);

        // Begin renderpass
        renderpass->begin(cmd, targetFBO);

        // Update dynamic viewport + scissor state
        commandBuffer->setViewport(targetFBO);
        commandBuffer->setScissor(targetFBO);

        // Update Shader-Descriptor-Set using the previous framebuffer(s) as the input
        VkDescriptorImageInfo imageInfo;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.sampler     = sampler->get();

        for (uint32_t i = 0; i < imageViews.size(); i++)
        {
            imageInfo.imageView = imageViews[i]->get();
            shader->setTexture("Input" + std::to_string(i), imageInfo);
        }

        // Bind Shader-Pipeline
        shader->bind(cmd);

        // Draw the fullscreen-quad
        vkCmdDraw(cmd, 3, 1, 0, 0);

        renderpass->end(cmd);
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    void SinglePostProcessStep::setupFramebuffer(uint32_t width, uint32_t height)
    {
        VkImageUsageFlags colorUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | 
                                       VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        // Create a framebuffer from the given renderpass
        framebuffer = new Framebuffer(VulkanBase::getDevice(), renderpass, width, height, { colorUsage });
    }

    //---------------------------------------------------------------------------
    //  OnSizeChanged - Callback
    //---------------------------------------------------------------------------

    // Called from the Post-Processing-Renderer if the window-size has changed
    void SinglePostProcessStep::onSizeChanged(float newWidth, float newHeight)
    {
        if (framebuffer != nullptr)
        {
            delete framebuffer;
            uint32_t width  = static_cast<uint32_t>(newWidth * resolutionModifier);
            uint32_t height = static_cast<uint32_t>(newHeight * resolutionModifier);
            setupFramebuffer(width, height);
        }
    }

    //---------------------------------------------------------------------------
    //  GaussianBlur11x1 - Class
    //---------------------------------------------------------------------------

    GaussianBlur9x1::GaussianBlur9x1(Renderpass* renderpass, 
                                     const std::string& shaderName, bool horizontal,
                                     float strength, float blurScale, float resolutionModifier)
        : SinglePostProcessStep(renderpass, shaderName, "/shaders/post_process/gaussian_blur9x1", resolutionModifier)
    {
        shader->setInt("horizontal", (int)horizontal);
        shader->setFloat("blurStrength", strength);
        shader->setFloat("blurScale", blurScale);
    }

    GaussianBlur9x1::GaussianBlur9x1(Renderpass* renderpass, const std::string& shaderName,
                                      uint32_t width, uint32_t height, bool horizontal, float strength, float blurScale)
        : SinglePostProcessStep(renderpass, width, height, shaderName, "/shaders/post_process/gaussian_blur9x1")
    {
        shader->setInt("horizontal", (int)horizontal);
        shader->setFloat("blurStrength", strength);
        shader->setFloat("blurScale", blurScale);
    }

    //---------------------------------------------------------------------------
    //  FXAA - Class
    //---------------------------------------------------------------------------

    FXAA::FXAA(Renderpass* renderpass, const std::string& shaderName, float spanMax, float reduceMin, float reduceMul)
        : SinglePostProcessStep(renderpass, shaderName, "/shaders/post_process/fxaa")
    {
        shader->setFloat("spanMax", spanMax);
        shader->setFloat("reduceMin", reduceMin);
        shader->setFloat("reduceMul", reduceMul);
    }

    //---------------------------------------------------------------------------
    //  ContrastChanger - Class
    //---------------------------------------------------------------------------

    ContrastChanger::ContrastChanger(Renderpass* renderpass, const std::string& shaderName, float contrast)
        : SinglePostProcessStep(renderpass, shaderName, "/shaders/post_process/contrast")
    {
        shader->setFloat("contrast", contrast);
    }

    //---------------------------------------------------------------------------
    //  RadialBlur - Class
    //---------------------------------------------------------------------------

    RadialBlur::RadialBlur(Renderpass* renderpass, const std::string& shaderName,
                           float scale, float strength, const Vec2f& origin, float resModifier)
        : SinglePostProcessStep(renderpass, shaderName, "/shaders/post_process/radial_blur", resModifier)
    {
        setScale(scale);
        setStrength(strength);
        setOrigin(origin);
    }

    //---------------------------------------------------------------------------
    //  FOG - Class
    //---------------------------------------------------------------------------

    FOG::FOG(Renderpass* renderpass, const std::string& shaderName,
             float zNear, float zFar, float density, float gradient, const Color& color, float resModifier)
        : SinglePostProcessStep(renderpass, shaderName, "/shaders/post_process/fog", resModifier)
    {
        shader->setFloat("zNear", zNear);
        shader->setFloat("zFar", zFar);
        shader->setFloat("density", density);
        shader->setFloat("gradient", gradient);
        shader->setColor("fogColor", color);
    }

    // Record all commands into the given cmd which renders this post-process-step
    // Some stages might use the original scene-framebuffer, thats why it is passed separately too
    void FOG::record(CommandBuffer* cmd, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer)
    {
        SinglePostProcessStep::record(cmd, { inputs[0]->getColorView(), sceneFramebuffer->getDepthView() });
    }

    //---------------------------------------------------------------------------
    //  Tonemap
    //---------------------------------------------------------------------------

    Tonemap::Tonemap(Renderpass* renderpass, const std::string& shaderName,
                     float exposure, float gamma, float resModifier)
        : SinglePostProcessStep(renderpass, shaderName, "/shaders/post_process/tonemap", resModifier)
    {
        shader->setFloat("exposure", exposure);
        shader->setFloat("gamma", gamma);
    }

    //---------------------------------------------------------------------------
    //  CameraDirtTexture
    //---------------------------------------------------------------------------

    CameraDirtTexture::CameraDirtTexture(Renderpass* renderpass, const std::string& shaderName, float _invisSpeedFactor, float resModifier)
        : SinglePostProcessStep(renderpass, shaderName, "/shaders/post_process/camera_dirt_texture", resModifier), 
          invisSpeedFactor(_invisSpeedFactor)
    {
        setTexture(TEXTURE_GET(TEX_DEFAULT_CAMERA_DIRT));
        shader->setFloat("visibility", 1.0f);
        lightShafts = PostProcessingRenderer::getLightShafts();
    }

    void CameraDirtTexture::update(float delta)
    {
        // Try to find it again if it failed in the constructor
        if (!lightShafts)
        {
            lightShafts = PostProcessingRenderer::getLightShafts();
            if(!lightShafts)
                Logger::Log("CameraDirtTexture::update(): Can't find light-shaft shader, but"
                           " it must be enabled for the CameraDirtTexture post process", LOGTYPE_ERROR);
        }

        const Vec2f& sunScreenPos = lightShafts->getSunScreenPos();
        setVisibility(sunScreenPos);
    }

    // Calculate visibility of texture based on how far the sun is in the center
    void CameraDirtTexture::setVisibility(const Vec2f& sunScreenPos)
    {
        // Distance from sun to screen-center
        float distance = (sunScreenPos - centerOnScreen).magnitude();

        float visibility = 1.0f - (distance * invisSpeedFactor);
        Mathf::clamp(visibility, 0.0f, 1.0f);

        shader->setFloat("visibility", visibility);
    }

    //---------------------------------------------------------------------------
    //  SunDisk - Class
    //---------------------------------------------------------------------------

    SunDisk::SunDisk(Renderpass* renderpass, const std::string& shaderName, float resModifier)
        : SinglePostProcessStep(renderpass, shaderName, "/shaders/post_process/sun", resModifier, PipelineType::Sun, false),
          lightDirection(0, -1.0f, 1.0f)
    {
        setTexture(TEXTURE_GET(TEX_DEFAULT_SUN));
        setColor(Color::WHITE);

        // Create the renderbuffer + imageview for the first time
        onSizeChanged((float)VulkanBase::getFinalWidth(), (float)VulkanBase::getFinalHeight());

        sunTransform.scale = Vec3f(130, 130, 1);
        sunDiskMesh = MESH(MODEL_QUAD_DEFAULT);
    }

    void SunDisk::setTexture(TexturePtr tex)
    { 
        tex.isValid() ? shader->setTexture("diffuse", tex) : shader->setTexture("diffuse", TEXTURE_GET(TEX_DEFAULT_SUN)); 
    }

    void SunDisk::onSizeChanged(float newWidth, float newHeight)
    {
        static VkImageUsageFlags colorUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
                                              | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        uint32_t width = static_cast<uint32_t>(newWidth * resolutionModifier);
        uint32_t height = static_cast<uint32_t>(newHeight * resolutionModifier);
        img = std::unique_ptr<VulkanImage>(new VulkanImage(VulkanBase::getDevice(), Vec2ui(width, height),
                                           renderpass->getColorAttachments()[0].format, colorUsage));
        view = std::shared_ptr<VulkanImageView>(new VulkanImageView(VulkanBase::getDevice(), *img, VK_IMAGE_VIEW_TYPE_2D));
    }


    void SunDisk::update(float delta)
    {
        Camera* camera = RenderingEngine::getCamera();

        // Sun-Quad should always look to the camera and be placed far enough away
        sunTransform.lookAt(camera->getTransform().position);
        float zFar = camera->getZFar() * 0.75f;
        sunTransform.position = camera->getTransform().position + (-lightDirection * zFar);

        // Calculate screen pos of the sun used by several other post-processing techniques
        Vec4f eyePos = camera->getViewProjection() * Vec4f(sunTransform.position, 1.0f);
        if (eyePos.w() < 0.0f)
        {
            // Sun behind the camera, make the screen pos arbitrary large
            screenPos = Vec2f(100.0f,100.0f);
        }
        else
        {
            Vec2f ndc = Vec2f(eyePos.x() / eyePos.w(), eyePos.y() / eyePos.w());
            screenPos = Vec2f((ndc.x() + 1) / 2.0f, (ndc.y() + 1) / 2.0f);
        }
    }

    void SunDisk::record(CommandBuffer* commandBuffer, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer)
    {
        // Delete old framebuffer referencing a depth-buffer from last frame
        if (framebuffer) {
            delete framebuffer;
            framebuffer = nullptr;
        }

        VkCommandBuffer cmd = commandBuffer->get();

        // Create a framebuffer referencing the color-buffer and the scene depth-buffer
        framebuffer = new Framebuffer(VulkanBase::getDevice(), renderpass, img->getWidth(), img->getHeight(), 
                                      { view }, sceneFramebuffer->getDepthView());

        // Begin renderpass
        renderpass->begin(cmd, framebuffer);

        // Update dynamic viewport + scissor state
        commandBuffer->setViewport(framebuffer);
        commandBuffer->setScissor(framebuffer);

        // Bind Shader-Pipeline
        shader->bind(cmd);

        // Now render the sun-disk
        RenderingEngine::getCamera()->bind(cmd, shader->getPipelineLayout());
        shader->pushConstant(cmd, 0, sizeof(Mat4f), &sunTransform.getTransformationMatrix());
        sunDiskMesh->bind(cmd);
        sunDiskMesh->draw(cmd);

        renderpass->end(cmd);
    }

    //---------------------------------------------------------------------------
    //  CombineFilter - SuperClass
    //---------------------------------------------------------------------------

    CombineFilter::CombineFilter(Renderpass* renderpass, const std::string& shaderName, const std::string& filePath)
        : SinglePostProcessStep(renderpass, shaderName, filePath)
    {}

    //---------------------------------------------------------------------------
    //  BloomCombineFilter - Class
    //---------------------------------------------------------------------------

    BloomCombineFilter::BloomCombineFilter(Renderpass* renderpass, const std::string& shaderName, float strength)
        : CombineFilter(renderpass, shaderName, "/shaders/post_process/bloom_combine_filter")
    {
        shader->setFloat("combineStrength", strength);
    }

    //---------------------------------------------------------------------------
    //  DepthOfFieldCombineFilter - Class
    //---------------------------------------------------------------------------

    DepthOfFieldCombineFilter::DepthOfFieldCombineFilter(Renderpass* renderpass,
                                                         const std::string& shaderName,
                                                         float focusThreshold, float transitionDistance)
        : CombineFilter(renderpass, shaderName, "/shaders/post_process/doa_combine_filter")
    {
        shader->setFloat("focusThreshold", focusThreshold);
        shader->setFloat("transitionDistance", transitionDistance);
    }

    //---------------------------------------------------------------------------
    //  MultiplePostProcessStep - Class
    //---------------------------------------------------------------------------

    // Record all commands into the given cmd which renders this post-process-step
    void MultiplePostProcessStep::record(CommandBuffer* cmd, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer)
    {
        Framebuffer* lastFramebuffer = inputs[0];
        for (auto& postProcessStep : postProcessSteps)
        {
            // Record Post-Process commands into the given cmd using the given framebuffer as the input
            postProcessStep->record(cmd, { lastFramebuffer }, sceneFramebuffer);

            // Next Post-Process step should use the previous framebuffer as input
            lastFramebuffer = postProcessStep->getOutputFramebuffer();
        }
        // Combine the FBO's
        combineFilter->record(cmd, { inputs[0], lastFramebuffer }, sceneFramebuffer);
    }

    //---------------------------------------------------------------------------
    //  Bloom - Class
    //---------------------------------------------------------------------------

    Bloom::Bloom(Renderpass* renderpass, float combineStrength, float blurResMod)
        : MultiplePostProcessStep()
    {
        postProcessSteps.push_back(new SinglePostProcessStep(renderpass, "Bloom#BrightFilter", "/shaders/post_process/bright_filter"));
        postProcessSteps.push_back(new GaussianBlur9x1(renderpass, "Bloom#HBlur1", true,  1.0f, 1.0f, blurResMod));
        postProcessSteps.push_back(new GaussianBlur9x1(renderpass, "Bloom#VBlur1",  false, 1.0f, 1.0f, blurResMod));
        //postProcessSteps.push_back(new GaussianBlur9x1(renderpass, "Bloom#HBlur2", true,  2.0f, 1.0f, blurResMod / 2.0f));
        //postProcessSteps.push_back(new GaussianBlur9x1(renderpass, "Bloom#VBlur2", false, 2.0f, 1.0f, blurResMod / 2.0f));
        combineFilter = new BloomCombineFilter(renderpass, "Bloom", combineStrength);
    }

    HDRBloom::HDRBloom(Renderpass* renderpass, float combineStrength, float blurResMod, float brightnessThreshold)
        : MultiplePostProcessStep()
    {
        SinglePostProcessStep* bloomFilter = new SinglePostProcessStep(renderpass, "HDRBloom#BrightFilter", "/shaders/post_process/hdr_bloom_filter");
        bloomFilter->getShader()->setFloat("brightnessThreshold", brightnessThreshold);
        postProcessSteps.push_back(bloomFilter);
        postProcessSteps.push_back(new GaussianBlur9x1(renderpass, "HDRBloom#VBlur1", false, 1.0f, 1.0f, blurResMod));
        postProcessSteps.push_back(new GaussianBlur9x1(renderpass, "HDRBloom#HBlur1", true,  1.0f, 1.0f, blurResMod));
        postProcessSteps.push_back(new GaussianBlur9x1(renderpass, "HDRBloom#VBlur2", false, 1.0f, 1.0f, blurResMod));
        postProcessSteps.push_back(new GaussianBlur9x1(renderpass, "HDRBloom#HBlur3", true,  1.0f, 1.0f, blurResMod));
        postProcessSteps.push_back(new GaussianBlur9x1(renderpass, "HDRBloom#VBlur3", false, 1.0f, 1.0f, blurResMod));
        postProcessSteps.push_back(new GaussianBlur9x1(renderpass, "HDRBloom#HBlur2", true,  1.0f, 1.0f, blurResMod));
        combineFilter = new BloomCombineFilter(renderpass, "HDRBloom", combineStrength);
    }

    //---------------------------------------------------------------------------
    //  DepthOfField - Class
    //---------------------------------------------------------------------------

    DepthOfField::DepthOfField(Renderpass* renderpass, float focusThreshold, float transitionDistance)
        : MultiplePostProcessStep()
    {
        postProcessSteps.push_back(new GaussianBlur9x1(renderpass, "DepthOfField#HBlur1", true,  1.0f, 1.0f, 0.75f));
        postProcessSteps.push_back(new GaussianBlur9x1(renderpass, "DepthOfField#VBlur1", false, 1.0f, 1.0f, 0.75f));

        combineFilter = new DepthOfFieldCombineFilter(renderpass, "DepthOfField", focusThreshold, transitionDistance);
    }

    void DepthOfField::update(float delta)
    {
        combineFilter->getShader()->setFloat("zNear", RenderingEngine::getCamera()->getZNear());
        combineFilter->getShader()->setFloat("zFar", RenderingEngine::getCamera()->getZFar());
    }

    // Record all commands into the given cmd which renders this post-process-step
    void DepthOfField::record(CommandBuffer* cmd, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer)
    {
        Framebuffer* lastFramebuffer = inputs[0];
        for (auto& postProcessStep : postProcessSteps)
        {
            // Record Post-Process commands into the given cmd using the given framebuffer as the input
            postProcessStep->record(cmd, { lastFramebuffer }, sceneFramebuffer);

            // Next Post-Process step should use the previous framebuffer as input
            lastFramebuffer = postProcessStep->getOutputFramebuffer();
        }
        // Combine the depth-image with the blurred-scene-image and the normal scene-image
        combineFilter->record(cmd, { sceneFramebuffer->getDepthView(), lastFramebuffer->getColorView(), inputs[0]->getColorView() });
    }

    //---------------------------------------------------------------------------
    //  LightShaft - Class
    //---------------------------------------------------------------------------

    // How the rendering works:
    // 1.) Render Sun-Disk using the scene depth-buffer (SunDisk - SinglePostProcessingStep)
    // 2.) Apply Radial-Blur to that result (RadialBlur - SinglePostProcessingStep)
    // 3.) Add the Radial-Blur result to the scene-framebuffer (combineFilter)
    LightShafts::LightShafts(Renderpass* renderpass, Renderpass* clearColorLoadDepth)
        : MultiplePostProcessStep()
    {
        sunDisk = new SunDisk(clearColorLoadDepth, "LightShafts#SunDisk", 1.0f);
        postProcessSteps.push_back(sunDisk);

        radialBlur = new RadialBlur(renderpass, "LightShafts#RadialBlur", 1.0f, 1.0f, Vec2f(), 0.5f);
        postProcessSteps.push_back(radialBlur);

        combineFilter = new BloomCombineFilter(renderpass, "LightShafts", 1.0f);
    }

    void LightShafts::update(float delta)
    {
        sunDisk->update(delta);
        radialBlur->update(delta);

        radialBlur->setOrigin(sunDisk->getScreenPos());
    }

    //---------------------------------------------------------------------------
    //  LensFlare Class
    //---------------------------------------------------------------------------

    LensFlare::LensFlare(ShaderPtr shader, TexturePtr _tex, float _scale)
        : tex(_tex), scale(_scale)
    {
        material = MATERIAL(shader);
        setTexture(tex);
        setVisibility(0.0f);
    }

    void LensFlare::bind(VkCommandBuffer cmd) { material->bind(cmd); }
    void LensFlare::setTexture(TexturePtr texture){ material->setTexture("tex", texture); }
    void LensFlare::setVisibility(float visibility) { material->setFloat("visibility", visibility); }
    bool LensFlare::isVisible() { return material->getFloat("visibility") != 0.0f; }

    Mat4f LensFlare::getTransform()
    {
        Vec2f ndcPos = screenPos - Vec2f(0.5f,0.5f);
        Vec3f translation = Vec3f(ndcPos.x(), ndcPos.y(), 0.0f);

        float xScale = scale;
        float yScale = xScale * ((float)Window::getWidth() / Window::getHeight());
        Vec3f scale = Vec3f(xScale, yScale, 1.0f);

        return Mat4f::trs(translation, Quatf::identity, scale);
    }

    //---------------------------------------------------------------------------
    //  LensFlares Class
    //---------------------------------------------------------------------------

    LensFlares::LensFlares(Resource<Shader> lensFlareShader, const std::vector<FlareInfo>& lensFlareInfos,
                           float _spacing, float _invisSpeedFactor)
        : spacing(_spacing), invisSpeedFactor(_invisSpeedFactor), screenPosition(-9999.0f,-9999.0f)
    {
        for (auto& lensFlareInfo : lensFlareInfos)
        {
            TexturePtr lensFlareTexture = TEXTURE(lensFlareInfo.filePath);
            LensFlare* newLensFlare = new LensFlare(lensFlareShader, lensFlareTexture, lensFlareInfo.scale);
            lensFlares.push_back(newLensFlare);
        }
    }

    LensFlares::~LensFlares()
    {
        for(auto& lensFlare : lensFlares)
            delete lensFlare;
    }

    void LensFlares::update(float delta)
    {
        Vec2f toCenter = centerOnScreen - screenPosition;
        float brightness = 1 - (toCenter.magnitude() * invisSpeedFactor);
        brightness = brightness < 0 ? 0 : brightness;

        // Calculate position for every lens-flare
        for (unsigned int i = 0; i < lensFlares.size(); i++)
        {
            lensFlares[i]->setVisibility(brightness);

            if (brightness > 0)
            {
                Vec2f flarePosition = screenPosition + (toCenter * (i * spacing));
                lensFlares[i]->setScreenPos(flarePosition);
            }
        }
    }

    //---------------------------------------------------------------------------
    //  LensFlareRenderer Class
    //---------------------------------------------------------------------------

    LensFlareRenderer::LensFlareRenderer(Renderpass* _renderpass, const std::string& shaderName)
        : renderpass(_renderpass), quadMesh(MESH(MODEL_QUAD_DEFAULT))
    {
        ShaderParams params(shaderName, "/shaders/post_process/lens_flare", PipelineType::AlphaBlend, renderpass);
        shader = SHADER(params);
    }

    LensFlareRenderer::~LensFlareRenderer()
    { 
        // Lens-Flare vector has to be empty. All Lens-Flares gets deleted when the component gets deleted
        assert(lensFlares.empty());
    }

    LensFlares* LensFlareRenderer::addNewLensFlares(const std::vector<FlareInfo>& lensFlareInfos, float spacing, float invisSpeedFactor)
    {
        shader->setActive(true);

        LensFlares* newLensFlares = new LensFlares(shader, lensFlareInfos, spacing, invisSpeedFactor);
        lensFlares.push_back(newLensFlares);

        return newLensFlares;
    }

    void LensFlareRenderer::removeLensFlares(LensFlares* pLensFlares)
    {
        lensFlares.erase( std::remove(lensFlares.begin(), lensFlares.end(), pLensFlares), lensFlares.end() );
        delete pLensFlares;

        if(lensFlares.empty()) shader->setActive(false);
    }

    void LensFlareRenderer::update(float delta)
    {
        for(auto& lensFlare : lensFlares)
            lensFlare->update(delta);
    }

    void LensFlareRenderer::record(CommandBuffer* commandBuffer, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer)
    {
        outputFramebuffer = inputs[0];

        // Do nothing if no lens-flares are present
        if(lensFlares.empty()) return;

        VkCommandBuffer cmd = commandBuffer->get();

        // Draw lens-flares
        renderpass->begin(cmd, outputFramebuffer);
        commandBuffer->setViewport(outputFramebuffer);
        commandBuffer->setScissor(outputFramebuffer);

        shader->bind(cmd);
        quadMesh->bind(cmd);

        // Render every lens-flare texture on a quad
        for (auto& lensFlares : lensFlares)
        {
            if(!lensFlares->isVisible())
                continue;

            for (auto& flare : lensFlares->getLensFlares())
            {
                shader->pushConstant(cmd, 0, sizeof(Mat4f), &flare->getTransform());
                flare->bind(cmd);
                quadMesh->draw(cmd);
            }
        }

        renderpass->end(cmd);
    }

    //---------------------------------------------------------------------------
    //  SSAO
    //---------------------------------------------------------------------------

    SSAO::SSAO()
    {

    }

    SSAO::~SSAO()
    {

    }

}