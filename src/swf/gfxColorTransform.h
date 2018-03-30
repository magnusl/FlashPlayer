#ifndef _GFXCOLORTRANSFORM_H_
#define _GFXCOLORTRANSFORM_H_

namespace swf
{
namespace gfx
{

struct ColorTransform {
    float MultTerms[4]; /**< multiplication terms */
    float AddTerms[4];  /**< addition terms */
};

}
}

#endif