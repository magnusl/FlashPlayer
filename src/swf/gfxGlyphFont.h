#ifndef _GFXGLYPHFONT_H_
#define _GFXGLYPHFONT_H_

#include <string>
#include "gfxShape.h"
#include <memory>
#include <stdint.h>

namespace swf
{
namespace gfx
{

struct GlyphFont
{
    std::string                                 FontName;
    std::vector<std::shared_ptr<gfx::Shape_t> > Glyphs;
    std::vector<int16_t>                        FontAdvanceTable;
    std::vector<uint16_t>                       CodeTable;

    std::shared_ptr<gfx::Shape_t> getGlyph(size_t index, int16_t &) const;
};

}

}

#endif