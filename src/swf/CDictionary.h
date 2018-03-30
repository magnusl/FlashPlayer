#ifndef _CDICTIONARY_H_
#define _CDICTIONARY_H_

#include "gfxShape.h"
#include "swfAction.h"
#include "gfxGlyphFont.h"
#include "swfText.h"

namespace swf
{

class CDictionary_Impl;
/**
 * SWF character dictionary that contains all the characters defined
 * in a ShockWave Flash (SWF) file.
 */
class CDictionary
{
public:
    CDictionary();
    virtual ~CDictionary();

    typedef enum {
        CHAR_SHAPE,
        CHAR_MOVIECLIP,
        CHAR_INVALID,
        CHAR_GLYPHFONT,
        CHAR_GLYPHTEXT
    } CharType_t;

    CharType_t getCharacterType(uint16_t) const;

    // Shapes
    bool addShape(std::shared_ptr<gfx::Shape_t>, uint16_t);
    std::shared_ptr<gfx::Shape_t> getShape(uint16_t);

    // Sprites/MovieClips
    bool addMovieClip(std::shared_ptr<swf::MovieClip>, uint16_t);
    std::shared_ptr<swf::MovieClip> getMovieClip(uint16_t);

    // glyph fonts
    bool addGlyphFont(std::shared_ptr<gfx::GlyphFont>, uint16_t);
    std::shared_ptr<gfx::GlyphFont> getGlyphFont(uint16_t);

    // glyph text
    bool addGlyphText(std::shared_ptr<GlyphText_t>, uint16_t);
    std::shared_ptr<GlyphText_t> getGlyphText(uint16_t);

private:
    CDictionary_Impl * impl;
};

}

#endif