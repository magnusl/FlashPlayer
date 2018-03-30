#ifndef _GFXGRADIENT_H_
#define _GFXGRADIENT_H_

#include <stdint.h>

namespace swf
{
namespace gfx
{

struct GradientPoint_t
{
    float   Color[4];
    uint8_t Ratio;

    bool operator ==(const GradientPoint_t & a_Point) const {
        if (Ratio != a_Point.Ratio) {
            return false;
        }
        for(size_t i = 0; i < 4; ++i) {
            if (Color[i] != a_Point.Color[i]) {
                return false;
            }
        }
        return true;
    }
};

struct Gradient_t {
    GradientPoint_t Points[16];
    uint8_t         Count;

    bool operator ==(const Gradient_t & a_Gradient) const {
        if (Count == a_Gradient.Count) {
            for(size_t i = 0; i < Count; ++i) {
                if (!(Points[i] == a_Gradient.Points[i])) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }
};

}
}

#endif