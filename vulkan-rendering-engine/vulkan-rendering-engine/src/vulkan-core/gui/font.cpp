#include "font.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Font::Font(const FontParams& params)
        : Texture(params), m_fontSize(params.fontSize)
    {
        m_sampler->setFilter(FILTER_LINEAR, FILTER_LINEAR);
        m_sampler->setMaxAnisotropy(1.0f);
        m_sampler->setMipmapMode(MIPMAP_MODE_NEAREST);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Calculate the text width in pixels from the given text based on this font-family and font-size
    int Font::getTextWidth(const std::string& text) const
    {
        float textWidth = 0;
        for (auto letter : text)
            textWidth += m_fontAtlas->getCharInfo(letter).ax;
        return static_cast<uint32_t>(textWidth);
    }

    // Calculate the text height in pixels from the given text (return the greatest height of the letters)
    int Font::getTextHeight(const std::string& text) const
    {
        float textHeight = 0;
        for (auto letter : text)
        {
            if (textHeight < m_fontAtlas->getCharInfo(letter).bh)
                textHeight = m_fontAtlas->getCharInfo(letter).bh;
        }
        return static_cast<uint32_t>(textHeight);
    }

}