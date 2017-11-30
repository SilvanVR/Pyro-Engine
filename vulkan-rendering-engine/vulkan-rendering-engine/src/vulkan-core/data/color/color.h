#ifndef COLOR_H_
#define COLOR_H_

#include "build_options.h"
#include "math/math_interface.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Color Class
    //---------------------------------------------------------------------------

    class Color : public Vec4f
    {
    public:
        Color();
        Color(std::string hex);
        Color(const Vec3f& color);
        Color(const Vec3f& color, float a);
        Color(float r, float g, float b);
        Color(float r, float g, float b, float a);
        ~Color() {};

        Vec3f getRGB() const { return Vec3f(r(), g(), b()); }

        // Return the greatest of the three r-g-b components (used to calculate the range of a light)
        float getMax();

        static Color WHITE;
        static Color BLACK;
        static Color RED;
        static Color GREEN;
        static Color BLUE;
        static Color YELLOW;
        static Color VIOLET;
        static Color TURQUOISE;
        static Color GREY;
        static Color ORANGE;

        // Operator overloading
        Color operator+(const Color& c) const;
        Color operator*(float f) const;
        bool operator==(const Color& c) const;
        bool operator!=(const Color& c) const;

    private:
        float hexToRGB(const std::string& hex);
    };

}



#endif // !COLOR_H_