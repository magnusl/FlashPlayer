#ifndef _GFXBEZIER_H_
#define _GFXBEZIER_H_

#include "gfxPoint.h"

namespace swf
{
namespace gfx
{

template<class T>
class Bezier
{
public:
    Bezier(const Point_t<T> & p0, const Point_t<T> & control, const Point_t<T> & p1) :
        m_p0(p0), m_control(control), m_p1(p1)
    {
    }

    Point_t<T> Get(float t)
    {
        float dt = 1.0f - t;
        return (m_p0 * dt * dt) + (m_control*2.0f*dt*t) + (m_p1 * (t*t));
    }

protected:
    Point_t<T> m_p0, m_control, m_p1;
};

}
}

#endif