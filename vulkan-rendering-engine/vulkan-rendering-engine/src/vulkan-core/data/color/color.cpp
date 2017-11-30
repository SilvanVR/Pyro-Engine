#include "color.h"


namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    Color Color::WHITE(1, 1, 1);
    Color Color::BLACK(0, 0, 0);
    Color Color::RED(1, 0, 0);
    Color Color::GREEN(0, 1, 0);
    Color Color::BLUE(0, 0, 1);
    Color Color::YELLOW(1, 1, 0);
    Color Color::VIOLET(1, 0, 1);
    Color Color::TURQUOISE(0, 1, 1);
    Color Color::GREY(0.5f, 0.5f, 0.5f);
    Color Color::ORANGE(1.0f, 0.5f, 0);

    //---------------------------------------------------------------------------
    //  Constructors
    //---------------------------------------------------------------------------

    Color::Color() : Vec4f(1, 1, 1, 1)
    {
    }

    Color::Color(const Vec3f& col) : Vec4f(col, 1)
    {
    }

    Color::Color(const Vec3f& col, float a) : Vec4f(col, a)
    {
    }

    Color::Color(float r, float g, float b) : Vec4f(r, g, b, 1)
    {
    }

    Color::Color(float r, float g, float b, float a) : Vec4f(r, g, b, a)
    {
    }

    Color::Color(std::string hex)
    {
        if (hex[0] != '#' || hex.length() != 7)
        {
            Logger::Log("Color::Color(): Given string '" + hex + "' is not a valid hexadecimal number."
                        " Using #FFFFFF (White) instead", LOGTYPE_WARNING);
            hex = "#FFFFFF";
        }
        // Cut of "#" character
        hex = hex.erase(0, 1);

        r() = hexToRGB(hex.substr(0, 2)) * 1/255.0f;
        g() = hexToRGB(hex.substr(2, 2)) * 1/255.0f;
        b() = hexToRGB(hex.substr(4, 2)) * 1/255.0f;
        a() = 1.0f;
    }


    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    // Return the greatest of the three r-g-b components (used to calculate the range of a light)
    float Color::getMax()
    {
        float max;

        max = r() > g() ? r() : g();
        max = max > b() ? max : b();

        return max;
    }

    //---------------------------------------------------------------------------
    //  Operator Overloading
    //---------------------------------------------------------------------------

    Color Color::operator+(const Color& c) const
    {
        return Color(r() + c.r(), g() + c.g(), b() + c.b(), a() + c.a());
    }

    Color Color::operator*(float f) const
    {
        return Color(r() * f, g() * f, b() * f, a() * f);
    }

    bool Color::operator==(const Color& c) const
    {
        return r() == c.r() && g() == c.g() && b() == c.b() && a() == c.a();
    }

    bool Color::operator!=(const Color& c) const
    {
        return !(*this == c);
    }

    //---------------------------------------------------------------------------
    //  Private Methods
    //---------------------------------------------------------------------------

    float Color::hexToRGB(const std::string& hex)
    {
        int hexLength = static_cast<int>(hex.length());
        double dec = 0;

        for (int i = 0; i < hexLength; ++i)
        {
            char b = hex[i];

            if (b >= 48 && b <= 57)
                b -= 48;
            else if (b >= 65 && b <= 70)
                b -= 55;

            dec += b * pow(16, ((hexLength - i) - 1));
        }
        return (float)dec;
    }

}