#include "freetype_loader.h"

#ifdef FREETYPE_LIB

#include "vulkan-core/gui/font.h"
#include "file_system/vfs.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Pyro
{

    Font* FreetypeLoader::loadFont(const FontParams& params)
    {
        static const int maxGlyphs = 256;

        std::string physicalPath = VFS::resolvePhysicalPath(params.filePath);

        FT_Library ft;
        if (FT_Init_FreeType(&ft))
            Logger::Log("FREETYPE: Could not init FreeType library!", LOGTYPE_ERROR);

        FT_Face face;
        if (FT_New_Face(ft, physicalPath.c_str(), 0, &face))
            Logger::Log("FREETYPE: Failed to load font: " + physicalPath, LOGTYPE_ERROR);

        // calculate the width dynamically based on the given height
        FT_Set_Pixel_Sizes(face, 0, params.fontSize);

        // Temporary custom Glyph-Object which pass the Glyph-Data from FreeType to the font-atlas
        Glyph glyphs[maxGlyphs];

        for (int c = 0; c < maxGlyphs; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                Logger::Log("FREETYPE: Failed to load Glyph: " + c, LOGTYPE_WARNING);
                continue;
            }

            auto glyph = face->glyph;
            const uint32_t size = glyph->bitmap.width * glyph->bitmap.rows;
            glyphs[c].c = c;
            glyphs[c].width = glyph->bitmap.width;
            glyphs[c].height = glyph->bitmap.rows;

            // Allocate memory for the glyph data and copy it
            glyphs[c].data = new unsigned char[size];
            memcpy(glyphs[c].data, face->glyph->bitmap.buffer, size);

            // Set all necessary fields
            glyphs[c].charInfo.ax = static_cast<float>(glyph->advance.x >> 6);
            glyphs[c].charInfo.ay = static_cast<float>(glyph->advance.y >> 6);

            glyphs[c].charInfo.bw = static_cast<float>(glyph->bitmap.width);
            glyphs[c].charInfo.bh = static_cast<float>(glyph->bitmap.rows);

            glyphs[c].charInfo.bl = static_cast<float>(glyph->bitmap_left);
            glyphs[c].charInfo.bt = static_cast<float>(glyph->bitmap_top);
        }

        // Create font object
        Font* pFont = new Font(params);

        // Create the new font-atlas with the given width and height. Automatically creates a bigger one if the size isnt enough
        pFont->m_fontAtlas = std::make_shared<TextureAtlas>(128, 128);

        // Skip the first 32 glyphs, add the rest to the font-atlas
        for (int c = 32; c < maxGlyphs; c++)
            pFont->m_fontAtlas->addGlyph(glyphs[c]);

        // Delete glyph data in the temporary glyph-objects after creating a gigantic texture for all glyphs in the font-atlas
        for (int c = 0; c < maxGlyphs; c++)
            delete glyphs[c].data;

        // Get Kerning for every characters if present and store it in the hash-map in the Texture-Atlas
        bool hasKerning = FT_HAS_KERNING(face) != 0 ? true : false;
        if (hasKerning)
        {
            for (int c = 0; c < maxGlyphs; c++)
            {
                for (int c2 = maxGlyphs; c2 > c; c2--)
                {
                    FT_UInt glyph_index = FT_Get_Char_Index(face, c);
                    FT_UInt glyph_index2 = FT_Get_Char_Index(face, c2);
                    FT_Vector kerning;
                    FT_Get_Kerning(face, glyph_index, glyph_index2, FT_KERNING_DEFAULT, &kerning);

                    int kerningPixels = kerning.x >> 6;

                    if (kerningPixels != 0)
                        pFont->m_fontAtlas->addKerning(c, c2, kerningPixels);
                }
            }
        }

        // Save information about format and width / height / size
        uint32_t dataSize = pFont->m_fontAtlas->getDataSize();
        pFont->m_format = VK_FORMAT_R8_UNORM;
        pFont->m_mipmaps.push_back({ pFont->m_fontAtlas->getWidth(), pFont->m_fontAtlas->getHeight(), dataSize });

        // Upload to GPU
        void* pData = pFont->m_fontAtlas->getData();
        pFont->uploadDataToGPU(pData, dataSize);

        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        return pFont;
    }

}

#endif //!FREETYPE_LIB