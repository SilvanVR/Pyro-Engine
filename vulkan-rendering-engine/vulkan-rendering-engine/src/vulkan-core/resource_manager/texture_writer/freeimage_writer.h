#ifndef FREEIMAGE_WRITER_H_
#define FREEIMAGE_WRITER_H_

#include "build_options.h"

#ifdef FREEIMAGE_LIB


namespace Pyro
{

    class FreeImageWriter
    {

    public:
        // Write the given pixels in a file (all common formats are supported with freeimage)
        static void writeImage(const std::string& filename, const std::vector<unsigned char>& pixels,
                               const Vec2ui& resolution, const uint32_t& bitsPerPixel);
    };

}


#endif // !FREEIMAGE_LIB

#endif // !FREEIMAGE_WRITER_H_

