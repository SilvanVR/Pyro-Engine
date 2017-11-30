#ifndef FONT_H_
#define FONT_H_

#include "vulkan-core/data/material/texture/texture.h"
#include "font_atlas.hpp"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Structs
    //---------------------------------------------------------------------------

    struct FontParams : public TextureParams
    {
        uint32_t fontSize;

        FontParams() {}
        FontParams(const std::string& name, const std::string& filePath, uint32_t _fontSize)
            : TextureParams(filePath, name), fontSize(_fontSize) {}
    };

    //---------------------------------------------------------------------------
    //  Font Class
    //---------------------------------------------------------------------------

    class Font : public Texture
    {
        friend class FreetypeLoader; // Allow to set fontAtlas and create a font

    public:
        // Calculate the text width in pixels from the given text
        int getTextWidth(const std::string& text) const;

        // Calculate the text height in pixels from the given text (return the greatest height of the letters)
        int getTextHeight(const std::string& text) const;

        int             getFontSize()   const { return m_fontSize; }
        TextureAtlas*   getAtlas()      const { return m_fontAtlas.get(); }

    private:
        // One gigantic texture for all the glyphs
        std::shared_ptr<TextureAtlas> m_fontAtlas;
        int m_fontSize;

        Font(const FontParams& params);
        ~Font() {}
    };

}



#endif // !FONT_H_
