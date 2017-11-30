#ifndef GUI_RENDERER_H_
#define GUI_RENDERER_H_

#include "vulkan-core/resource_manager/resource.hpp"
#include "vulkan-core/pipelines/shaders/shader.h"
#include "vulkan-core/vulkan_base.h"
#include "vulkan-core/gui/gui.h"
#include "../sub_renderer.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  GUIRenderer-Class
    //---------------------------------------------------------------------------

    class GUIRenderer : public SubRenderer
    {
        Vec4f*                      mapped = nullptr;   // Pointer to mapped vertex buffer
        uint32_t                    numQuads;           // Num Quads in the Vertex-Buffer

        // Vertex-Buffer containing all quads (letters) - one for each frame-data
        std::vector<std::unique_ptr<VulkanVertexBuffer>> vertexBuffers;

        // Pointers to the host-mappable vertex-buffers
        std::vector<Vec4f*>         buffPointers;

        // Renderpass which loads the color attachment, instead of clearing it
        Renderpass*                 renderpass = nullptr;

        //TODO: USE ONLY ONE SHADER. Possible? With Distance-Field-Fonts?
        Resource<Shader>            fontShader;
        Resource<Shader>            guiShader;

        // The GUIRenderer contains a list of GUIBufferElements, which contains necessary information for rendering
        struct GUIBufferElements
        {
            GUIElement*     element;    // The GUI-Element itself
            uint32_t        bufferPos;  // The position of the component in the vertex-buffer
            uint32_t        numQuads;   // The number of quads this component has (e.g. image's has always 1, text's differs)
            bool            font;       // True: Element is a font. Used to differentiate between font and images. TODO: USE 1 PIPE for BOTH and remove this field here.
        };

    public:
        GUIRenderer(RenderingEngine* renderingEngine);
        ~GUIRenderer();

        // Record the draw-commands in a command-buffer given by the index
        void recordCommandBuffer(uint32_t frameDataIndex, Framebuffer* framebuffer) override;

        // Called from the Rendering-Engine when the window size has changed.
        void onSizeChanged(float newWidth, float newHeight) override;

    private:
        // Put all gui - data on the gpu
        void updateGPU(uint32_t frameDataIndex);

        // Setup the renderpass
        void setupRenderpass(const VkFormat& colorFormat);

        // Put a text into the vertex buffer
        void addTextToBuffer(GUIText* text);

        // Put a image (one quad) into the vertex buffer
        void addImageToBuffer(GUIElement* image);

        // Add a quad to the vertex buffer
        void addQuad(float x1, float y1, float x2, float y2, float u1 = 0, float u2 = 0, float v1 = 1, float v2 = 1);

        //---------------------------------------------------------------------------
        //  Static Private Methods - GUI Functions
        //---------------------------------------------------------------------------

        // Store all GUI-Objects, the position in the vertex-buffer and the amount of quads 
        std::vector<GUIBufferElements> guiBufferElements;

        // Allow the gui-class to add themselve in the constructor to this class
        friend class GUI;

        // All GUI's in the scene
        static std::vector<GUI*> guis;

        static void addGUI(GUI* gui);
        static void removeGUI(GUI* gui);

    };

}


#endif // !GUI_RENDERER_H_