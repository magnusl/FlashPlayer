#ifndef _GFXTRIANGLE_H_
#define _GFXTRIANGLE_H_

namespace swf
{
namespace gfx
{

struct Triangle_t {
    Triangle_t(uint32_t a_P1, uint32_t a_P2, uint32_t a_P3) : p1(a_P1), p2(a_P2), p3(a_P3)
    {}

    Triangle_t()
    {}

    uint32_t p1, p2, p3;
};

}
}

#endif