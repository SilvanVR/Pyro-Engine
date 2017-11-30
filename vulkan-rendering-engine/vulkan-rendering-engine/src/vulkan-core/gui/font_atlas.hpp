#ifndef FONT_ATLAS_H_
#define FONT_ATLAS_H_

#include <map>

namespace Pyro
{


    // Character-Metrics & UV-Coordinates
    struct CharacterInfo {
        float ax;           // advance.x
        float ay;           // advance.y

        float bw;           // bitmap.width;
        float bh;           // bitmap.rows;

        float tw;           // width in texture coordinates
        float th;           // height in texture coordinates

        float bl;           // bitmap_left;
        float bt;           // bitmap_top;

        Vec2f texCoords;    // x/y offset of glyph in texture coordinates
        Vec2f pixelCoords;  // x/y offset of glyph in pixel coordinates
    };

    // Glyph object. Temporary object to pass the necessary glyph data from FreeType to the texture atlas.
    struct Glyph
    {
        char            c;          // The character this glyph belongs to
        unsigned char*  data;       // The pixel data for this glyph
        uint32_t        width;      // The width in pixels of this glyph
        uint32_t        height;     // The height in pixels of this glyph

        CharacterInfo   charInfo;   // Character-Metrics & UV-Coordinates
    };

    // Texture Atlas, which basically creates one gigantic texture with all glyphs from a font.
    class TextureAtlas
    {
        friend class FreetypeLoader;
    private:
        uint32_t width;             // Width of this texture
        uint32_t height;            // Height of this texture
        uint32_t maxSize;           // Data-Size of this texture

        uint32_t paddingWidth;      // Padding between glyphs in a row in pixels
        uint32_t paddingHeight;     // Padding between all glyphs in a row to the next row in pixels

        uint32_t maxHeightRow;      // Used to calculate the amount of data jumped to the next row
        uint32_t currentWidth;      // Current width where dataPtr points to in pixels and used to check when to jump to the next row
        uint32_t currentHeight;     // Current height where dataPtr points to in pixels

        char startGlyph;            // The first Glyph in this atlas
        uint32_t numGlyphs;         // The number of glyphs in this atlas

        unsigned char* data;        // Pointer to the beginning of the data block
        unsigned char* dataPtr;     // Pointer to where the next glyph should be lay down

        // Contains information about the glyphs. Used to render them correctly via different offsets
        std::map<char, Glyph> glyphHashMap;

        // Contains information about the kerning of two glyphs
        std::map<int, int> kerningHashMap;

    public:
        // Create a texture-atlas with the given width & height. Automatically creates a bigger one in "addGlyph()" if its not enough.
        TextureAtlas(uint32_t _width, uint32_t _height)
            : width(_width), height(_height), maxHeightRow(0), currentWidth(0), currentHeight(0), numGlyphs(0), startGlyph(-1), paddingWidth(4), paddingHeight(2)
        {
            maxSize = width * height;
            data = new unsigned char[maxSize];
            memset(data, 0, maxSize);
            dataPtr = data;
        };

        ~TextureAtlas()
        {
            delete data;
        }

        // Add a glyph to this atlas. Automatically creates a bigger texture if the texture is not big enough.
        void addGlyph(Glyph& glyph)
        {
            // Jump to next row if the current-width would exceed the max-width of one row
            if (currentWidth + (glyph.width + paddingWidth) > this->width)
            {
                currentHeight += (maxHeightRow + paddingHeight);    // Increment current height
                dataPtr = data + (currentHeight * width);           // Jump to the next row.
                maxHeightRow = 0;                                   // Reset the max-height parameter
                currentWidth = 0;                                   // Reset the current-width parameter
            }

            // Check if enough space is available for the glyph. If not, increase the size of the texture, allocate new memory and add all the old glyphs to the new one.
            if ((dataPtr + (glyph.height * width)) > (data + maxSize))
            {
                // Reset everything
                maxHeightRow = 0;
                currentWidth = 0;
                currentHeight = 0;
                delete[] data;

                // Increase Witdh and Height
                width = 2 * width;
                height = 2 * height;
                maxSize = width * height;

                // Allocate new memory
                data = new unsigned char[maxSize];
                memset(data, 0, maxSize);
                dataPtr = data;

                // Add all old glyphs to the new texture
                uint32_t oldNumGlyphs = numGlyphs;
                numGlyphs = 0; // Reset numGlyphs
                for (uint32_t c = startGlyph; c < startGlyph + oldNumGlyphs; c++)
                {
                    Glyph glyph = glyphHashMap[(char)c];
                    glyphHashMap.erase((char)c);
                    addGlyph(glyph);
                }
            }

            // Remember the start glyph if this function has to create a bigger texture
            if (startGlyph == -1)
                startGlyph = glyph.c;

            // Increment number of glyphs
            numGlyphs++;

            // Store the information where to find the glyph in tex-coords (0-1) and pixel-coords
            glyph.charInfo.pixelCoords = Vec2f((float)currentWidth, (float)currentHeight);
            glyph.charInfo.texCoords = Vec2f((float)currentWidth / width, (float)currentHeight / height);

            // Calculate width/height of the glyph in texture coordinates
            glyph.charInfo.tw = glyph.charInfo.bw / width;
            glyph.charInfo.th = glyph.charInfo.bh / height;

            // Store the info where to find the glyph in the texture-atlas etc.
            glyphHashMap[glyph.c] = glyph;

            // Copy glyph data into the texture
            unsigned char* beginGlyph = dataPtr;
            unsigned char* glyphData = glyph.data;
            for (uint32_t i = 0; i < glyph.height; i++)
            {
                beginGlyph = dataPtr + i * width; // Jump to the next row
                for (uint32_t j = 0; j < glyph.width; j++)
                {
                    *beginGlyph = *glyphData;
                    beginGlyph++;
                    glyphData++;
                }
            }

            // Height of this glyph greater then the current max height?
            if (glyph.height > maxHeightRow)
                maxHeightRow = glyph.height;

            // Increment the data-ptr and the current width
            if (currentWidth + (glyph.width + paddingWidth) < this->width)
            {
                dataPtr += glyph.width + paddingWidth;
                currentWidth += glyph.width + paddingWidth;
            }
        }

        // Return the character info for the given character. Contains information like texture coords. Glyph width & height and more.
        CharacterInfo& getCharInfo(char c)
        {
            return glyphHashMap[c].charInfo;
        }

        // Add a kerning value for two character to this font-atlas
        void addKerning(char c1, char c2, int kerning)
        {
            kerningHashMap[hashCode(c1, c2)] = kerning;
        }

        // Return a kerning value for the given two characters
        int getKerning(char c1, char c2)
        {
            if (kerningHashMap.count(hashCode(c1, c2)) != 0)
                return kerningHashMap[hashCode(c1, c2)];
            else
                return 0;
        }

        // Return the width & height of the used texture for this atlas
        uint32_t getWidth() { return width; }
        uint32_t getHeight() { return height; }

        // Return the pointer to the beginning of the font-texture
        unsigned char* getData() { return data; }

        // Return the size of the font-texture in bytes
        uint32_t getDataSize() { return maxSize; }

    private:
        // Calculates a hashcode for two characters. Used as a key for the kerning Hashmap.
        int hashCode(char c1, char c2)
        {
            const int BASE = 21;
            const int MULTIPLIER = 31;

            int hashCode = BASE;

            hashCode = MULTIPLIER * hashCode + c1;
            hashCode = MULTIPLIER * hashCode + c2;

            return hashCode;
        }
    };

}

#endif // !FONT_ATLAS_H_
