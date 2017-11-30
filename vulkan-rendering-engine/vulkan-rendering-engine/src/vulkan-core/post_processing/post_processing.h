#ifndef POST_PROCESSING_H_
#define POST_PROCESSING_H_

#include "build_options.h"

#include "vulkan-core/pipelines/framebuffers/framebuffer.h"
#include "vulkan-core/scene_graph/nodes/transform.h"
#include "vulkan-core/resource_manager/resource.hpp"
#include "vulkan-core/pipelines/shaders/shader.h"

// Handles all Post-Processing effects
// Render the scene on a full-screen quad using a unlimited amount of different post-processing shaders

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Forward declarations
    //---------------------------------------------------------------------------
    
    class LightShafts;
    class VulkanSampler;
    class Mesh;

    //---------------------------------------------------------------------------
    //  PostProcessStep class
    //---------------------------------------------------------------------------

    // Represents one post-processing technique like Bloom, Depth-Of-Field etc.
    class PostProcessStep
    {
    public:
        PostProcessStep(){}
        virtual ~PostProcessStep() {}

        virtual void update(float delta) {}

        // Record all commands into the given cmd which renders this post-process
        // Some stages might use the original scene-framebuffer, thats why it is passed separately too
        virtual void record(CommandBuffer* cmd, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer) = 0;

        // Return the framebuffer in which this post-process-step renders
        virtual Framebuffer* getOutputFramebuffer() = 0;

        // Return whether this post-processing step is active
        virtual bool isActive() = 0;

        // Called from the Post-Processing-Renderer if the window-size has changed
        virtual void onSizeChanged(float newWidth, float newHeight) = 0;
    };

    //---------------------------------------------------------------------------
    //  SinglePostProcessStep class
    //---------------------------------------------------------------------------

    // Represents one post-process-step which renders a full-screen-quad with a shader.
    class SinglePostProcessStep : public PostProcessStep
    {
    public:
        // Constructor which creates a own framebuffer in this class and recreates it
        // when window size changes. The size is always window-size * resolutionModifier.
        // IF 0.0f is passed in for the "resolutionModifier", no framebuffer gets created within this class.
        // It should be passed to the "record"-function then
        SinglePostProcessStep(Renderpass* renderpass, const std::string& shaderName, const std::string& filePath,
                              float resolutionModifier = 1.0f, PipelineType pipelineType = PipelineType::PostProcess,
                              bool createFramebuffer = true);
        // Constructor which create a framebuffer with the specified size, which dont gets recreated.
        SinglePostProcessStep(Renderpass* renderpass, const uint32_t& width, const uint32_t& height, 
                             const std::string& shaderName, const std::string& filePath);
        virtual ~SinglePostProcessStep();

        // Record all commands into the given cmd which renders this post-process-step.
        void record(CommandBuffer* cmd, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer) override;

        // Record all commands into the given cmd which renders this post-process-step
        // in the target FBO using the inputs-FBO as inputs
        void record(CommandBuffer* cmd, Framebuffer* targetFBO, const std::vector<Framebuffer*>& inputs);

        // Record all commands into the given cmd which renders this post-process-step using the renderpass / framebuffer from this class
        void record(CommandBuffer* cmd, const std::vector<std::shared_ptr<VulkanImageView>>& imageViews);

        // Record all commands into the given cmd which renders this post-process-step
        // in the target FBO using the image-views as inputs
        void record(CommandBuffer* cmd, Renderpass* renderpass, Framebuffer* targetFBO, const std::vector<std::shared_ptr<VulkanImageView>>& imageViews);

        // Return the framebuffer in which this post-process-step renders
        Framebuffer* getOutputFramebuffer() override { return framebuffer; }

        // Return the shader used by this post-process step
        ShaderPtr getShader() { return shader; }

        // Return whether this post-processing step is active
        bool isActive() { return shader->isActive(); }

        // Called from the Post-Processing-Renderer if the window-size has changed
        void onSizeChanged(float newWidth, float newHeight);

        void setSampler(const VulkanSampler* newSampler) { sampler = newSampler; }

    protected:
        Renderpass*             renderpass;     // Post-Process Renderpass
        ShaderPtr               shader;         // Post-Process Shader
        Framebuffer*            framebuffer;    // Post-Process framebuffer in which the shader will render
        const VulkanSampler*    sampler;        // Sampler used to sample from the input-attachments

        // When the window-size changes, framebuffer gets recreated with 
        // size * resolutionModifier if this class has a framebuffer
        float                   resolutionModifier;

        void setupFramebuffer(uint32_t width, uint32_t height);
    };

    //---------------------------------------------------------------------------
    //  MultiplePostProcessStep class
    //---------------------------------------------------------------------------

    // Represents a class, which combines two or more textures into one
    class CombineFilter : public SinglePostProcessStep
    {
    public:
        CombineFilter(Renderpass* renderpass, const std::string& shaderName, const std::string& filePath);
        virtual ~CombineFilter() {}
    };

    // Represents a post-process-step which can contain multiple SinglePostProcessSteps, 
    // e.g. for rendering two separate textures and combine them subsequently.
    // Used for advanced techniques which requires more than one input.
    class MultiplePostProcessStep : public PostProcessStep
    {
    public:
        MultiplePostProcessStep() : PostProcessStep() {}
        ~MultiplePostProcessStep() { for (auto& step : postProcessSteps) { delete step; } delete combineFilter; }

        // Combine two or more textures into one FBO using the combineFilter
        virtual void record(CommandBuffer* cmd, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer) override;

        // Return whether this post-processing step is active
        bool isActive() override { return combineFilter->isActive(); }

        // Return the framebuffer in which the combine-filter renders
        Framebuffer* getOutputFramebuffer() override { return combineFilter->getOutputFramebuffer(); }

        // Call method on all post-process steps
        void onSizeChanged(float newWidth, float newHeight) {
            for (auto& step : postProcessSteps) { step->onSizeChanged(newWidth, newHeight); }
            combineFilter->onSizeChanged(newWidth, newHeight);
        }

    protected:
        // All SinglePostProcess-Steps in this advanced technique
        std::vector<PostProcessStep*>   postProcessSteps;

        // Combine-Filter which combine several textures into one
        CombineFilter*  combineFilter;
    };

    //---------------------------------------------------------------------------
    //  SinglePostProcessStep SubClasses
    //---------------------------------------------------------------------------

    class GaussianBlur9x1 : public SinglePostProcessStep
    {
    public:
        GaussianBlur9x1(Renderpass* renderpass, const std::string& shaderName,
                        bool horizontal = true, float strength = 1.0f, float blurScale = 1.0f, float resolutionModifier = 1.0f);
        GaussianBlur9x1(Renderpass* renderpass, const std::string& shaderName,
                        uint32_t width, uint32_t height, bool horizontal = true, float strength = 1.0f, float blurScale = 1.0f);
    };

    class FXAA : public SinglePostProcessStep
    {
    public:
        FXAA(Renderpass* renderpass, const std::string& shaderName,
             float spanMax = 4.0f, float reduceMin = 0.1f, float reduceMul = 0.25f);
    };

    class ContrastChanger : public SinglePostProcessStep
    {
    public:
        ContrastChanger(Renderpass* renderpass, const std::string& shaderName, float contrast = 0.0f);
    };

    class RadialBlur : public SinglePostProcessStep
    {
    public:
        RadialBlur(Renderpass* renderpass, const std::string& shaderName,
                   float scale = 0.35f, float strength = 1.0f, const Vec2f& origin = Vec2f(), float resModifier = 1.0f);

        void setScale(float scale) { shader->setFloat("radialBlurScale", scale); }
        void setStrength(float strength) { shader->setFloat("radialBlurStrength", strength); }
        void setOrigin(const Vec2f& origin) { shader->setVec2f("radialOrigin", origin); }
    };

    class FOG : public SinglePostProcessStep
    {
    public:
        FOG(Renderpass* renderpass, const std::string& shaderName,
            float zNear, float zFar, float density = 0.007f, float gradient = 1.5f, const Color& color = Color::GREY, float resModifier = 1.0f);

        // overriden because the shaders samples from the scene-DEPTH attachment
        void record(CommandBuffer* cmd, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer) override;
    };

    class Tonemap : public SinglePostProcessStep
    {
    public:
        Tonemap(Renderpass* renderpass, const std::string& shaderName,
                float exposure = 1.0f, float gamma = 2.2f, float resModifier = 1.0f);
    };

    class CameraDirtTexture : public SinglePostProcessStep
    {
        LightShafts* lightShafts;
        float invisSpeedFactor; // Higher values makes the texture faster invis

    public:
        CameraDirtTexture(Renderpass* renderpass, const std::string& shaderName, 
                          float invisSpeedFactor = 4.0f, float resModifier = 1.0f);

        void update(float delta) override;
        void setTexture(TexturePtr tex) { shader->setTexture("dirtTexture", tex); }

    private:
        void setVisibility(const Vec2f& sunScreenPos);
    };

    // Render the sun-texture on a quad placed near zFar using the scene-depthbuffer 
    // The resulting buffer should just contain the sun hidden by geometry if present
    class SunDisk : public SinglePostProcessStep
    {
        Transform                           sunTransform;
        Vec3f                               lightDirection;
        Vec2f                               screenPos;
        Resource<Mesh>                      sunDiskMesh;
        std::unique_ptr<VulkanImage>        img;
        std::shared_ptr<VulkanImageView>    view;

    public:
        SunDisk(Renderpass* renderpass, const std::string& shaderName, float resModifier = 1.0f);

        // Calculate position from sun near zFar and the screen position
        void update(float delta) override;

        const Vec2f& getScreenPos() const { return screenPos; }

        void record(CommandBuffer* cmd, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer) override;
        void onSizeChanged(float newWidth, float newHeight) override;

        void setColor(const Color& color) { shader->setColor("sunColor", color); }
        void setTexture(TexturePtr tex);
        void setLightDirection(const Vec3f& lightDir) { lightDirection = lightDir; }
        void setScale(const Vec3f& scale) { sunTransform.scale = scale; }
    };

    //---------------------------------------------------------------------------
    //  SinglePostProcessStep SubClasses - Combine Filters
    //---------------------------------------------------------------------------

    class BloomCombineFilter : public CombineFilter
    {
    public:
        BloomCombineFilter(Renderpass* renderpass, const std::string& shaderName, float strength = 1.0f);
    };

    class DepthOfFieldCombineFilter : public CombineFilter
    {
    public:
        DepthOfFieldCombineFilter(Renderpass* renderpass, const std::string& shaderName,
                                  float depthThreshold, float transitionDistance);
    };

    //---------------------------------------------------------------------------
    //  MultiplePostProcessStep SubClasses
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    //  Bloom Class
    //---------------------------------------------------------------------------

    class Bloom : public MultiplePostProcessStep
    {
    public:
        Bloom(Renderpass* renderpass, float combineStrength = 1.0f, float blurResMod = 0.5f);
        ~Bloom() {}
    };

    class HDRBloom : public MultiplePostProcessStep
    {
    public:
        HDRBloom(Renderpass* renderpass, float combineStrength = 1.0f, float blurResMod = 0.5f, float brightnessThreshold = 1.0f);
        ~HDRBloom() {}
    };


    //---------------------------------------------------------------------------
    //  Depth-Of-Field Class
    //---------------------------------------------------------------------------

    class DepthOfField : public MultiplePostProcessStep
    {
    public:
        DepthOfField(Renderpass* renderpass, float focusThreshold, float transitionDistance = 20);
        ~DepthOfField() {}

        // Override update method to readjust zFar/zFar in the shader
        void update(float delta) override;

        // Override record-method because we have to sample from the depth-attachment from the scene-fbo
        void record(CommandBuffer* cmd, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer) override;
    };

    //---------------------------------------------------------------------------
    //  LightShaft Class
    //---------------------------------------------------------------------------

    class LightShafts : public MultiplePostProcessStep
    {
        RadialBlur*         radialBlur;
        SunDisk*            sunDisk;

    public:
        LightShafts(Renderpass* renderpass, Renderpass* clearColorLoadDepth);
        ~LightShafts() { /* radialBlur + sunDisk deleted in destructor of super-class */ }

        void update(float delta) override;

        void toggleActive() { combineFilter->getShader()->toggleActive(); }
        void setActive(bool b){ combineFilter->getShader()->setActive(b); }
        void setColor(const Color& color) { sunDisk->setColor(color); }
        void setSunTexture(TexturePtr tex) { sunDisk->setTexture(tex); }
        void setLightDirection(const Vec3f& lightDir) { sunDisk->setLightDirection(lightDir); }
        void setScale(const Vec3f& scale) { sunDisk->setScale(scale); }
        void setRadialBlurStrength(float strength) { radialBlur->setStrength(strength); }
        void setRadialBlurScale(float scale) { radialBlur->setScale(scale); }
        const Vec2f& getSunScreenPos(){ return sunDisk->getScreenPos(); }
    };

    //---------------------------------------------------------------------------
    //  LensFlare + LensFlares Class
    //---------------------------------------------------------------------------

    struct FlareInfo
    {
        const char* filePath;
        float scale;
    };

    class LensFlare
    {
        MaterialPtr material;
        TexturePtr tex;
        Vec2f screenPos;
        float scale;

    public:
        LensFlare(ShaderPtr shader, TexturePtr tex, float scale);
        ~LensFlare() {}

        float getScale() { return scale; }
        Mat4f getTransform();

        void bind(VkCommandBuffer cmd);
        void setTexture(TexturePtr texture);
        void setScreenPos(const Vec2f& newPos){ screenPos = newPos; }
        void setVisibility(float visibility);
        bool isVisible();
    };

    class LensFlares
    {
    public:
        LensFlares(Resource<Shader> lensFlareShader, const std::vector<FlareInfo>& lensFlareInfos,
                   float spacing = 4.0f, float invisSpeedFactor = 1.0f);
        ~LensFlares();

        void update(float delta);

        void setScreenPosition(const Vec2f& newScreenPos){ screenPosition = newScreenPos; }
        const std::vector<LensFlare*> getLensFlares(){ return lensFlares; }
        bool isVisible(){ return lensFlares.empty() ? false : lensFlares.front()->isVisible(); }

    private:
        Vec2f screenPosition;                 // The screen position of the attached object - updated every frame

        std::vector<LensFlare*> lensFlares;   // Textures which will be rendered as quads on the screen
        float spacing;                        // The distance from a quad to the others
        float invisSpeedFactor;               // How fast the textures will become invisible
    };

    //---------------------------------------------------------------------------
    //  LensFlareRenderer Class
    //---------------------------------------------------------------------------

    class LensFlareRenderer : public PostProcessStep
    {
    private:
        std::vector<LensFlares*>    lensFlares;         // Lens-Flares
        Resource<Shader>            shader;             // LensFlare shader
        Framebuffer*                outputFramebuffer;  // Framebuffer, same as inputs[0]
        Resource<Mesh>              quadMesh;           // Mesh used to render the lens flares
        Renderpass*                 renderpass;         // Renderpass used to render the lens flares

    public:
        LensFlareRenderer(Renderpass* renderpass, const std::string& shaderName);
        ~LensFlareRenderer();

        LensFlares* addNewLensFlares(const std::vector<FlareInfo>& lensFlareInfos, float spacing, float invisSpeedFactor);
        void removeLensFlares(LensFlares* pLensFlares);

        void update(float delta) override;
        void record(CommandBuffer* cmd, const std::vector<Framebuffer*>& inputs, Framebuffer* sceneFramebuffer);

        Framebuffer* getOutputFramebuffer() { return outputFramebuffer; }
        void toggleActive(){ shader->toggleActive(); }
        bool isActive() { return shader->isActive(); }
        void onSizeChanged(float newWidth, float newHeight) {}
    };


    //---------------------------------------------------------------------------
    //  SSAO
    //---------------------------------------------------------------------------

    class SSAO : public MultiplePostProcessStep
    {
    public:
        SSAO();
        ~SSAO();

    private:
    };

}




#endif // !POST_PROCESSING_H_

