#ifndef FREETYPE_LOADER_H_
#define FREETYPE_LOADER_H_

#ifdef FREETYPE_LIB

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class Font;
    struct FontParams;

    //---------------------------------------------------------------------------
    //  FreetypeLoader Class
    //---------------------------------------------------------------------------

    class FreetypeLoader
    {
    public:
        static Font* loadFont(const FontParams& params);
    };

}

#endif // !FREETYPE_LIB

#endif // !FREETYPE_LOADER_H_
