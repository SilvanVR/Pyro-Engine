#ifndef RENDERER_H_
#define RENDERER_H_

#include "vulkan_base.h"

#include "scene_graph/nodes/camera/camera.h"
#include "data/material/texture/cubemap.h"
#include "sub_renderer/sub_renderer.h"
#include "pipelines/shaders/shader.h"
#include "data_types.hpp"

namespace Pyro
{

    enum class ERenderingMode
    {
        SOLID,
        UNLIT,
        LIT,
        WIREFRAME
    };

    //---------------------------------------------------------------------------
    //  Renderer class
    //---------------------------------------------------------------------------

    class RenderingEngine : public VulkanBase
    {
        enum SubRendererType
        {
            GUI,
            SHADOW,
            POSTPROCESS
        };

        friend class SubRenderer;
        friend class PostProcessingRenderer;
        friend class StaticObjectRenderer;
        friend class ShadowRenderer;
        friend class GUIRenderer;

    public:
        // Creates and Initializes the rendering-engine. Renders into the given Window.
        RenderingEngine(Window* window);

        // Creates and Initializes the rendering-engine. Renders in the specified dimensions.
        // Retrieve the rendered result via setRenderCallback()
        RenderingEngine(const Vec2ui& resolution);
        ~RenderingEngine();

        // Return the currently used camera for rendering
        static Camera* getCamera() { assert(camera != nullptr); return camera; }

        // Shorthand function for setRenderCallback(func); update(0); draw(); (Only for special cases)
        void draw(const std::function<void(const ImageData&)>& func) { setRenderCallback(func); update(0); draw(); }

        // Record command buffers, dispatch them to the gpu and (present the rendered image to the window)
        void draw();

        // Update Scene-Graph (with all Objects + Components)
        void update(float delta);

        // Reset the current state of the renderer (render-mode, irradiance-map etc.). Called when a scene gets loaded.
        void resetStateToDefault();

        // Change the rendering mode
        void setRenderingMode(ERenderingMode renderingMode);

        // Set the camera used for rendering 
        void setCamera(Camera* _camera) { camera = _camera; camera->setAspecRatio((float)get3DRenderWidth() / (float)get3DRenderHeight()); }

        // IBL Stuff
        void setAmbientIntensity(float ambientIntensity) { gBufferShader->setFloat("ambientIntensity", ambientIntensity); }
        float getAmbientIntensity() { return gBufferShader->getFloat("ambientIntensity"); }
        void setEnvMapSpecularIntensity(float specularIntensity) { gBufferShader->setFloat("specularIntensity", specularIntensity); }
        float getEnvMapSpecularIntensity() { return gBufferShader->getFloat("specularIntensity"); }
        void setUseIBL(bool useIBL) { gBufferShader->setInt("useIBL", useIBL ? 1 : 0); }
        bool isUsingIBL() { return gBufferShader->getInt("useIBL") == 0 ? false : true; }
        void setEnvironmentMap(CubemapPtr cubemap) { gBufferShader->setTexture("EnvironmentMap", cubemap); gBufferShader->setFloat("maxReflectionLOD", (float)(cubemap->numMips()-1)); }
        void setAmbientIrradianceMap(CubemapPtr cubemap) { gBufferShader->setTexture("IrradianceMap", cubemap); }
        void setBRDFLUT(TexturePtr tex) { gBufferShader->setTexture("BRDFLut", tex); }

        // FOG-Stuff
        // Changes the FOG-Color from the fog-post-processing shader AND the skybox-shader
        void setFOGIsActive(bool b);
        void toggleFOG();
        void setFogColor(const Color& fogColor);
        Color getFogColor();
        void setFOGDensity(float density);
        float getFOGDensity();
        void setFOGGradient(float gradient);
        float getFOGGradient();

        // Manipulate rendering
        void setTimeScale(float scale) { this->timeScale = scale; }
        float getTimeScale() { return timeScale; }
        float getAlphaDiscardThreshold(){ gBufferShader->getFloat("alphaDiscardThreshold"); }
        void setAlphaDiscardThreshold(float val){ gBufferShader->setFloat("alphaDiscardThreshold", val); }
        void setClearColor(const Color& color);
        void toggleShadows();
        void toggleNormalMapRendering();
        void toggleDispMapRendering();
        void toggleGUI();
        void toggleRenderAlbedo();
        void toggleRenderNormals();
        void setRenderBoundingBoxes(bool b);
        void toggleBoundingBoxes();

        // Attach an callback to this renderer. It will be called ONLY ONCE next time the rendering has been finished
        // If you want to get the data every frame call this function every frame
        void setRenderCallback(std::function<void(const ImageData&)> func);

        // Create a Buffer for an image which can be filled with data through "fillPreprocessBuffer". It will be rendered BEFORE the 3d-scene.
        void createPreProcessBuffer(const Vec2ui& size, VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM);
        void fillPreprocessBuffer(void* pixelData);

    private:
        static Camera*   camera;            // Camera used for rendering

        ERenderingMode   renderingMode;     // The Rendering mode currently used (LIT / WIREFRAME / SOLID)

        ShaderPtr       baseShader;         // Shader which is used for rendering all objects if enabled (wireframe/solid)
        ShaderPtr       solidShader;
        ShaderPtr       wireframeShader;

        ShaderPtr       gBufferShader;
        ShaderPtr       dirLightShader;
        ShaderPtr       pointLightShader;
        ShaderPtr       spotLightShader;

        std::map<SubRendererType, SubRenderer*> subRenderer; // All SubRenderer e.g. GUIRenderer, ShadowRenderer, PostProcessRenderer

        // Initialize everything
        void init();

        // Record all command-buffers from all renderers
        void recordCommandBuffers();

        // Record primary command-buffer which renders the scene
        void recordSceneCommandBuffer();

        // Record deferred commands into the given cmd
        void recordDeferredLightingCommands(VkCommandBuffer cmd);

        // Record forward-rendering commands into the given cmd
        void recordForwardCommands(VkCommandBuffer cmd);

        // Called if the window size changes
        void onSizeChanged() override;

        // If valid, it will be called when rendering has been finished this frame (ONLY ONCE)
        std::function<void(const ImageData&)> renderingFinishedCallback;

        // Transfer the rendered result into an host visible buffer, retrieve it and call the callback
        void getRenderedDataAndCallCallback(VulkanImage& renderedImage);

        // Modifies delta time for scene-graph updates
        float timeScale = 1.0f;

        // Stuff for preFill the Light-Acc Framebuffer with data (renders a fullscreen quad)
        bool                preProcessingEnabled    = false;
        ShaderPtr           preProcessShader;
        TexturePtr          preProcessTexture;
    };

}


#endif // !RENDERER_H_


