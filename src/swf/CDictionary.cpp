#include "CDictionary.h"
#include <map>

using namespace std;

namespace swf
{

class CDictionary_Impl
{
public:
    std::map<uint16_t, CDictionary::CharType_t> types;
    std::map<uint16_t, std::shared_ptr<gfx::Shape_t> > shapes;
    std::map<uint16_t, std::shared_ptr<swf::MovieClip> > sprites;
    std::map<uint16_t, std::shared_ptr<gfx::GlyphFont> > glyphFonts;
    std::map<uint16_t, std::shared_ptr<GlyphText_t> > glyphText;
};

CDictionary::CharType_t CDictionary::getCharacterType(uint16_t id) const
{
    if (impl) {
        map<uint16_t, CDictionary::CharType_t>::const_iterator it = impl->types.find(id);
        if (it == impl->types.end()) {
            return CDictionary::CHAR_INVALID;
        }
        return it->second;
    }
    return CDictionary::CHAR_INVALID;
}

CDictionary::CDictionary()
{
    impl = new (std::nothrow) CDictionary_Impl;
}

CDictionary::~CDictionary()
{
    delete impl;
}

bool CDictionary::addShape(std::shared_ptr<gfx::Shape_t> shape, uint16_t id)
{
    if (getCharacterType(id) != CDictionary::CHAR_INVALID) {
        return false;
    }
    impl->types[id]     = CHAR_SHAPE;
    impl->shapes[id]    = shape;
    return true;
}

std::shared_ptr<gfx::Shape_t> CDictionary::getShape(uint16_t id)
{
    map<uint16_t, std::shared_ptr<gfx::Shape_t> >::iterator it = impl->shapes.find(id);
    if (it != impl->shapes.end()) {
        return it->second;
    }
    return std::shared_ptr<gfx::Shape_t>();
}

bool CDictionary::addMovieClip(std::shared_ptr<swf::MovieClip> mc, uint16_t id)
{
    if (getCharacterType(id) != CDictionary::CHAR_INVALID) {
        return false;
    }
    impl->types[id]     = CHAR_MOVIECLIP;
    impl->sprites[id]   = mc;
    return true;
}

std::shared_ptr<swf::MovieClip> CDictionary::getMovieClip(uint16_t id)
{
    map<uint16_t, std::shared_ptr<swf::MovieClip> >::iterator it = impl->sprites.find(id);
    if (it != impl->sprites.end()) {
        return it->second;
    }
    return std::shared_ptr<swf::MovieClip>();
}

bool CDictionary::addGlyphFont(std::shared_ptr<gfx::GlyphFont> font, uint16_t id)
{
    if (getCharacterType(id) != CDictionary::CHAR_INVALID) {
        return false;
    }
    impl->types[id]         = CHAR_GLYPHFONT;
    impl->glyphFonts[id]    = font;
    return true;
}

std::shared_ptr<gfx::GlyphFont> CDictionary::getGlyphFont(uint16_t id)
{
    std::map<uint16_t, std::shared_ptr<gfx::GlyphFont> >::iterator it = impl->glyphFonts.find(id);
    if (it == impl->glyphFonts.end()) {
        return std::shared_ptr<gfx::GlyphFont>();
    }
    return it->second;
}

bool CDictionary::addGlyphText(std::shared_ptr<GlyphText_t> text, uint16_t id)
{
    if (getCharacterType(id) != CDictionary::CHAR_INVALID) {
        return false;
    }
    impl->types[id]         = CHAR_GLYPHTEXT;
    impl->glyphText[id]     = text;
    return true;
}

std::shared_ptr<GlyphText_t> CDictionary::getGlyphText(uint16_t id)
{
    std::map<uint16_t, std::shared_ptr<GlyphText_t> >::iterator it = impl->glyphText.find(id);
    if (it == impl->glyphText.end()) {
        return std::shared_ptr<GlyphText_t>();
    }
    return it->second;
}

}