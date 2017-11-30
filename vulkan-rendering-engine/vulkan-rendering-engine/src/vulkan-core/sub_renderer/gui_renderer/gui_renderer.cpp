#include "gui_renderer.h"

#include "vulkan-core/memory_management/vulkan_memory_manager.h"
#include "vulkan-core/resource_manager/resource_manager.h"
#include "vulkan-core/pipelines/renderpass/renderpass.h"
#include "vulkan-core/scene_graph/scene_graph.h"
#include "vulkan-core/vkTools/vk_tools.h"
#include "vulkan-core/rendering_engine.h"

#include <array>

namespace Pyro
{

    #define MAX_QUAD_COUNT 2048

    //---------------------------------------------------------------------------
    //  Static stuff
    //---------------------------------------------------------------------------

    std::vector<GUI*> GUIRenderer::guis;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    GUIRenderer::GUIRenderer(RenderingEngine* r)
        : SubRenderer(r, true)
    {
        // Vertex buffer containing all quads to be rendered (letters and/or images)
        VkDeviceSize bufferSize = MAX_QUAD_COUNT * sizeof(Vec4f) * 4; // 4 Vertices with X/Y/U/V = 16 * 4 = 64 Bytes per Quad

        for (int i = 0; i < r->numFrameResources; i++)
        {
            auto newBuffer = std::unique_ptr<VulkanVertexBuffer>(new VulkanVertexBuffer(
                                                                 device, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
            vertexBuffers.push_back(std::move(newBuffer));

            // Map GPU-Memory once. Unmapped automatically in destructor of the buffer-class.
            buffPointers.push_back((Vec4f*)vertexBuffers[i]->map());
        }

        setupRenderpass(r->getSurfaceFormat());

        // Create the shaders using the created renderpass
        ShaderParams params("GUI", "/shaders/gui", PipelineType::GUI, renderpass);
        guiShader = SHADER(params);

        ShaderParams params2("Font", "/shaders/font", PipelineType::GUI, renderpass);
        fontShader = SHADER(params2);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    GUIRenderer::~GUIRenderer()
    {
        delete renderpass;
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Put all gui - data on the gpu
    void GUIRenderer::updateGPU(uint32_t frameDataIndex)
    {
        // Reset everything
        guiBufferElements.clear();
        numQuads = 0;
        mapped = buffPointers[frameDataIndex];

        for (auto& gui : guis)
        {
            // Continue if this GUI is not enabled
            if (!gui->isActive())
                continue;

            for (auto& guiElem : gui->getGUIElements())
            {
                // Continue if this gui-element is not active
                if(!guiElem->isActive())
                    continue;

                // Add the gui-element to the vertex-buffer
                switch (guiElem->getType())
                {
                case GUIElement::TEXT:
                    addTextToBuffer(dynamic_cast<GUIText*>(guiElem)); break;
                case GUIElement::IMAGE:
                case GUIElement::BUTTON:
                    addImageToBuffer(guiElem); break;
                case GUIElement::SLIDER:
                    // Do Nothing. The Slider-Class itself contains 2 GUIImage, which will be rendered
                    break;
                }
            }
        }
    }

    // Record the draw-commands in a command-buffer given by the index
    void GUIRenderer::recordCommandBuffer(uint32_t frameDataIndex, Framebuffer* framebuffer)
    {
        // Update GPU-Buffer first. Recording the commands is dependant of those values.
        updateGPU(frameDataIndex);

        // Record commands for drawing all quads.
        cmdBuffers[frameDataIndex]->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        {
            VkCommandBuffer cmd = cmdBuffers[frameDataIndex]->get();

            // Begin renderpass
            renderpass->begin(cmd, framebuffer);
            
            // Update dynamic states
            cmdBuffers[frameDataIndex]->setViewport(framebuffer);
            cmdBuffers[frameDataIndex]->setScissor(framebuffer);

            // Bind vertex buffer containing all quads
            vertexBuffers[frameDataIndex]->bind(cmd, VERTEX_BUFFER_BIND_ID);

            // Render all GUI-Elements
            for (auto& elem : guiBufferElements)
            {
                // Bind appropriate pipeline
                if (elem.font)
                    fontShader->bind(cmd);
                else
                    guiShader->bind(cmd);

                // Bind material from that element
                elem.element->getMaterial()->bind(cmd);

                // Draw Quads from that component
                for (uint32_t j = elem.bufferPos; j < elem.bufferPos + elem.numQuads; j++)
                    vkCmdDraw(cmd, 4, 1, j * 4, 0);
            }

            renderpass->end(cmd);
        }
        cmdBuffers[frameDataIndex]->end();
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    // Setup the renderpass
    void GUIRenderer::setupRenderpass(const VkFormat& colorFormat)
    {
        VkAttachmentDescription colorAttachment;

        // color attachment
        colorAttachment.flags           = 0;
        colorAttachment.format          = colorFormat;
        colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Renderpass for gui
        renderpass = new Renderpass(device, { colorAttachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
    }

    // Put a text (one quad per letter) into the vertex buffer
    void GUIRenderer::addTextToBuffer(GUIText* text)
    {
        // Get screen-position of the text
        Vec2f penPosition = text->getAnchoredPosition((float)VulkanBase::getFinalWidth(), (float)VulkanBase::getFinalHeight());

        // Adapt position if the text is aligned
        switch (text->getAlign())
        {
        case TextAlign::RIGHT:
            penPosition.x() -= text->getWidth(); break;
        case TextAlign::CENTER:
            penPosition.x() -= text->getWidth() / 2; break;
        }

        // Save the buffer-position and the number of quads of this text-object
        guiBufferElements.push_back({ text, numQuads, static_cast<uint32_t>(text->getTextSize()), true });

        // Get the Texture-Atlas which stores the Glyphs of the Font in a texture
        TextureAtlas* charAtlas = text->getFont()->getAtlas();

        // Generate a uv mapped quad per char
        std::string& string = text->getText();
        Vec2f scale = text->getScale();
        for (unsigned int i = 0; i < string.size(); i++)
        {
            // Get Character-Information which contains Character-Metrics & UV-Coordinates
            CharacterInfo& charData = charAtlas->getCharInfo(string[i]);

            // Calculate Vertex-Positions. Take bearing into account
            float x1 = penPosition.x() + charData.bl * scale.x();
            float y1 = penPosition.y() - charData.bt * scale.y();

            float x2 = x1 + charData.bw * scale.x();
            float y2 = y1 + charData.bh * scale.y();

            // Calculate UV-Coordinates
            float u1 = charData.texCoords.x();
            float v1 = charData.texCoords.y();

            float u2 = charData.texCoords.x() + charData.tw;
            float v2 = charData.texCoords.y() + charData.th;

            // Create a Quad with the appropriate uv-coordinates
            addQuad(x1, y1, x2, y2, u1, v1, u2, v2);

            // Get Kerning from the font-atlas from this and the next character
            int kerning = 0;
            if (i < (string.size() - 1))
                kerning = charAtlas->getKerning(string[i], string[i + 1]);

            // Move the pen based on the kerning and scale of the letter
            penPosition.x() += (charData.ax + kerning) * scale.x();
        }
    }

    // Put a image (one quad) into the vertex buffer
    void GUIRenderer::addImageToBuffer(GUIElement* image)
    {
        // Get screen-position of the image
        Vec2f penPosition = image->getAnchoredPosition((float)VulkanBase::getFinalWidth(), (float)VulkanBase::getFinalHeight());

        // Save the buffer-position and the number of quads of this image-object
        guiBufferElements.push_back({ image, numQuads, 1, false });

        // Calculate Vertex-Positions
        float x = penPosition.x();
        float y = penPosition.y();

        float x2 = penPosition.x() + image->getWidth();
        float y2 = penPosition.y() + image->getHeight();

        // Create a Quad with the appropriate uv-coordinates and put it in the buffer
        addQuad(x, y, x2, y2, 0, 0, 1, 1);
    }

    // Add a quad to the vertex buffer. 
    // x1, x2, y1, y2 form the four vertices. (x1, y2) Bottom-Left, (x1, y1) Top-Left, (x2, y2) Bottom-Right, (x2, y1) Top-Right.
    // u1, v1, u2, v2 form the four uv-coordinates. (u1, v2) Bottom-Left, (u1, v1) Top-Left, (u2, v2) Bottom-Right, (u2, v1) Top-Right.
    inline void GUIRenderer::addQuad(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2)
    {
        // Check if enough space is available in the vertex-buffer
        assert(mapped != nullptr && numQuads + 1 <= MAX_QUAD_COUNT);

        float fbW = static_cast<float>(VulkanBase::getFinalWidth());
        float fbH = static_cast<float>(VulkanBase::getFinalHeight());

        // Transform the four vertices into NDC-Coordinates
        float x1NDC = (x1 / fbW * 2.0f) - 1.0f;
        float y1NDC = (y1 / fbH * 2.0f) - 1.0f;

        float x2NDC = (x2 / fbW * 2.0f) - 1.0f;
        float y2NDC = (y2 / fbH * 2.0f) - 1.0f;

        // Bottom-Left Vertex
        mapped->x() = x1NDC;
        mapped->y() = y2NDC;
        mapped->z() = u1;
        mapped->w() = v2;
        mapped++;

        // Top-Left Vertex
        mapped->x() = x1NDC;
        mapped->y() = y1NDC;
        mapped->z() = u1;
        mapped->w() = v1;
        mapped++;

        // Bottom-Right Vertex
        mapped->x() = x2NDC;
        mapped->y() = y2NDC;
        mapped->z() = u2;
        mapped->w() = v2;
        mapped++;

        // Top-Right Vertex
        mapped->x() = x2NDC;
        mapped->y() = y1NDC;
        mapped->z() = u2;
        mapped->w() = v1;
        mapped++;

        numQuads++;
    }


    // Called from the Rendering-Engine when the window size has changed.
    void GUIRenderer::onSizeChanged(float newWidth, float newHeight)
    {
        for (auto& gui : guis)
            gui->onSizeChanged(newWidth, newHeight);
    }

    //---------------------------------------------------------------------------
    //  Static Private Methods - GUI Functions
    //---------------------------------------------------------------------------

    void GUIRenderer::addGUI(GUI* gui)
    {
        guis.push_back(gui);
    }

    void GUIRenderer::removeGUI(GUI* gui)
    {
        removeObjectFromList(guis, gui);
    }

}