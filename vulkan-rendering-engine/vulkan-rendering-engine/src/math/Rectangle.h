/*
*  Rectangle - Class header file.
*
*  Date:       23.06.2016
*  Creator:    Silvan Hau
*/
#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

#include "util.h"  /* clamp              */

//---------------------------------------------------------------------------
//  Forward Declarations
//---------------------------------------------------------------------------
template <typename T> class Vec2;

namespace math
{
    //---------------------------------------------------------------------------
    //  Rectangle Class
    //---------------------------------------------------------------------------
    template <typename T>
    class Rectangle
    {
        T m_x;            // X - Position
        T m_y;            // Y - Position
        T m_width;        // Width of the Rectangle
        T m_height;       // Height of the Rectangle

    public:

        //Constructor
        explicit Rectangle() : m_x(0), m_y(0), m_width(0), m_height(0) {}
        explicit Rectangle(T _width, T _height) : m_x(0), m_y(0), m_width(_width), m_height(_height) {}
        explicit Rectangle(T _x, T _y, T _width, T _height) : m_x(_x), m_y(_y), m_width(_width), m_height(_height) {}

        //Operators
        bool        operator==      (const Rectangle<T> &)          const;
        bool        operator!=      (const Rectangle<T> &)          const;

        //Const member functions
        bool        inside          (const T& x, const T& y)        const;          //Check if a given position is inside the rectangle.
        Vec2<T>     getPosition     ()                              const;          //Return the position as a vec2.
        T           x               ()                              const;          //Return the X-Value from this Rectangle.
        T           y               ()                              const;          //Return the Y-Value from this Rectangle.
        T           width           ()                              const;          //Return the width from this Rectangle.
        T           height          ()                              const;          //Return the height from this Rectangle.
        //bool intersect(const Rectangle<T>& rect) const;

        template <typename T2>
        bool        inside          (const Vec2<T2>& point)          const;         //Check if a given position is inside the rectangle.
                        
        //Nonconst member functions 
        void        setPosition     (const T& x, const T& y);                       //Set the position from this Rectangle.
        T&          x               ();                                             //Return the First-Value of this vector by reference.
        T&          y               ();                                             //Return the Y-Value from this Rectangle by reference.
        T&          width           ();                                             //Return the width from this Rectangle by reference.
        T&          height          ();                                             //Return the height from this Rectangle by reference.

        template <typename T2>
        void        setPosition     (const Vec2<T2>& point);                        //Set the position from this Rectangle.


        //Static members

    };


    //---------------------------------------------------------------------------
    //  Operator Overloading
    //---------------------------------------------------------------------------

    template <typename T> inline
    bool Rectangle<T>::operator==(const Rectangle<T> & r) const
    {
        return x() == r.x() && y() == r.y() && width() == r.width() && height() == r.height();
    }

    template <typename T> inline
    bool Rectangle<T>::operator!=(const Rectangle<T> & r) const
    {
        return !(*this == r);
    }

    //---------------------------------------------------------------------------
    //  Const Member Functions
    //---------------------------------------------------------------------------

    // Check if a given position is inside the rectangle.
    template <typename T> inline
    bool Rectangle<T>::inside(const T& x, const T& y) const
    {
        return x > this->x() && x < (this->x() + this->width()) &&
               y > this->y() && y < (this->y() + this->height());
    }

    // Return the position
    template <typename T> inline
    Vec2<T> Rectangle<T>::getPosition() const
    {
        return Vec2<T>(x(), y());
    }

    // Check if a given position is inside the rectangle.
    template <typename T>
    template <typename T2> inline
    bool Rectangle<T>::inside(const Vec2<T2>& point) const
    {
        return point.x() > this->x() && point.x() < (this->x() + this->width()) &&
               point.y() > this->y() && point.y() < (this->y() + this->height());
    }

    //Return the X-Value from this Rectangle.
    template <typename T> inline
    T Rectangle<T>::x() const
    {
        return m_x;
    }

    //Return the Y-Value from this Rectangle.
    template <typename T> inline
    T Rectangle<T>::y() const
    {
        return m_y;
    }

    //Return the width from this Rectangle.
    template <typename T> inline
    T Rectangle<T>::width() const
    {
        return m_width;
    }

    //Return the height from this Rectangle.
    template <typename T> inline
    T Rectangle<T>::height() const
    {
        return m_height;
    }

    //---------------------------------------------------------------------------
    //  Non-const Member Functions
    //---------------------------------------------------------------------------

    // Set the position
    template <typename T> inline
    void Rectangle<T>::setPosition(const T& x, const T& y)
    {
        this->x() = x;
        this->y() = y;
    }

    // Set the position
    template <typename T> 
    template <typename T2> inline
    void Rectangle<T>::setPosition(const Vec2<T2>& point)
    {
        this->x() = static_cast<T>(point.x());
        this->y() = static_cast<T>(point.y());
    }

    //Return the X-Value from this Rectangle by reference.
    template <typename T> inline
    T& Rectangle<T>::x()
    {
        return m_x;
    }

    //Return the Y-Value from this Rectangle by reference.
    template <typename T> inline
    T& Rectangle<T>::y()
    {
        return m_y;
    }

    //Return the width from this Rectangle by reference.
    template <typename T> inline
    T& Rectangle<T>::width()
    {
        return m_width;
    }

    //Return the height from this Rectangle by reference.
    template <typename T> inline
    T& Rectangle<T>::height()
    {
        return m_height;
    }

}
#endif // _RECTANGLE_H_