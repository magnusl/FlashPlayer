#ifndef _SWFTEXT_H_
#define _SWFTEXT_H_

#include <stdint.h>
#include <vector>
#include "swfTypes.h"
#include <list>

namespace swf
{

struct GlyphEntry_t {
    uint32_t GlyphIndex;
    int32_t  GlyphAdvance;
};

struct GlyphTextSegment_t {
    std::vector<GlyphEntry_t>   Text;
    uint16_t    FontId;
    Color_t     Color;
    int16_t     XOffset, YOffset;
    uint16_t    TextHeight;
    uint8_t     HasFont     : 1;
    uint8_t     HasColor    : 1;
    uint8_t     HasYOffset  : 1;
    uint8_t     HasXOffset  : 1;
};

struct GlyphText_t {
    std::list<GlyphTextSegment_t>   Segments;
    swf::RECT                       TextBounds;
    swf::Matrix_Fixed2x3            TextMatrix;
};

}

#endif