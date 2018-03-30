#ifndef _GFXPOINT_H_
#define _GFXPOINT_H_

namespace swf
{
namespace gfx
{

/**
 * Point in 2D space.
 */
template<class T>
struct Point_t
{
    Point_t() : X(0), Y(0)
    {}

    Point_t(T a_X, T a_Y) : X(a_X), Y(a_Y)
    {}

    T X, Y;

    Point_t<T> operator *(T a_Value) const  { return Point_t<T>(X * a_Value, Y * a_Value); }
    Point_t<T> operator +(const Point_t<T> & a_Rhs) const { return Point_t<T>(X + a_Rhs.X, Y + a_Rhs.Y); }
    Point_t<T> operator -(const Point_t<T> & a_Rhs) const { return Point_t<T>(X - a_Rhs.X, Y - a_Rhs.Y); }
    bool operator==(const Point_t & a_Rhs) const { return (X == a_Rhs.X) && (Y == a_Rhs.Y); }
};

}
}

#endif