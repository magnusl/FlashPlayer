#include "CPlayer.h"
#include "swfDecode.h"
#include "gfxGlyphFont.h"

using namespace std;

namespace swf
{
namespace gfx
{

std::shared_ptr<gfx::Shape_t> GlyphFont::getGlyph(size_t index, int16_t & advance) const
{
    if (index >= Glyphs.size()) {
        return std::shared_ptr<gfx::Shape_t>();
    }
    if (index < FontAdvanceTable.size()) {
        advance = this->FontAdvanceTable[index];
    } else {
        advance = -1;
    }
    return Glyphs[index];
}

}

static std::shared_ptr<gfx::Shape_t> glyphToShape(swf::Shape & a_Glyph);

void CPlayer::DefineFont3(CTagDecoder & tag)
{
    uint16_t  FontID = tag.GetU16();
    uint8_t FontFlagsHasLayout      = tag.GetBits(1);
    uint8_t FontFlagsShiftJIS       = tag.GetBits(1);
    uint8_t FontFlagsSmallText      = tag.GetBits(1);
    uint8_t FontFlagsANSI           = tag.GetBits(1);
    uint8_t FontFlagsWideOffsets    = tag.GetBits(1);
    uint8_t FontFlagsWideCodes      = tag.GetBits(1);
    uint8_t FontFlagsItalic         = tag.GetBits(1);
    uint8_t FontFlagsBold           = tag.GetBits(1);

    uint8_t LanguageCode            = tag.GetU8();
    uint8_t FontNameLen             = tag.GetU8();

    std::string FontName;
    for(size_t i = 0; i < FontNameLen; ++i) {
        FontName += (char) tag.GetU8();
    }

    uint16_t NumGlyphs = tag.GetU16();
    if (!NumGlyphs) {
        throw std::runtime_error("Device font support not implemented.");
    }

    int32_t tableOffset = tag.TagOffset();
    std::vector<uint32_t> OffsetTable(NumGlyphs);
    if (FontFlagsWideOffsets) {
        for(size_t i = 0; i < NumGlyphs; ++i) OffsetTable[i] = tag.GetU32();
    } else {
        for(size_t i = 0; i < NumGlyphs; ++i) OffsetTable[i] = tag.GetU16();
    }

    uint32_t CodeTableOffset = FontFlagsWideOffsets ? tag.GetU32() : tag.GetU16();
    std::vector<swf::Shape> GlyphShapeTable(NumGlyphs);
    for(size_t i = 0; i < NumGlyphs;++i) 
    {
        tag.SetPosition(tableOffset + OffsetTable[i]);
        Decode<swf::Shape>(tag, GlyphShapeTable[i]);
    }

    shared_ptr<gfx::GlyphFont> font = make_shared<gfx::GlyphFont>();
    
    font->CodeTable.resize(NumGlyphs);
    for(size_t i = 0; i < NumGlyphs; ++i) {
        font->CodeTable[i] = tag.GetU16();
    }
    uint16_t FontAscent = FontFlagsHasLayout ? tag.GetU16() : 0;
    uint16_t FontDescent = FontFlagsHasLayout ? tag.GetU16() : 0;
    uint16_t FontLeading = FontFlagsHasLayout ? tag.GetS16() : 0;

    if (FontFlagsHasLayout) {
        font->FontAdvanceTable.resize(NumGlyphs);
        for(size_t i = 0; i < NumGlyphs; ++i) {
            font->FontAdvanceTable[i] = tag.GetS16();
        }
    }
    font->Glyphs.resize(NumGlyphs);
    for(size_t i = 0; i < NumGlyphs; ++i) {
        if (!(font->Glyphs[i] = glyphToShape(GlyphShapeTable[i]))) {
            throw std::runtime_error("Failed to convert glyph to shape.");
        }
        // glyphs defined with DefineFont3 has 20 times the precision
        font->Glyphs[i]->ScaleFactor    = 400;
        font->Glyphs[i]->UserData       = -1;
    }
    // Now add the font to the dictionary.
    if (!swfDictionary.addGlyphFont(font, FontID)) {
        throw std::runtime_error("Failed to add glyph font to dictionary.");
    }
}

static std::shared_ptr<gfx::Shape_t> glyphToShape(swf::Shape & a_Glyph)
{
    std::shared_ptr<gfx::Shape_t> shape = std::make_shared<gfx::Shape_t>();
    std::vector<gfx::Edge_t> edges;
    recordsToEdges(a_Glyph.Shapes, edges);

    shape->Glyph = true;

    std::vector<gfx::LineStyle_t> LineStyles;
    std::vector<gfx::FillStyle_t> FillStyles(1);
    FillStyles[0].FillType = gfx::FillStyle_t::SolidFill;
    FillStyles[0].u.Color.r = 0;
    FillStyles[0].u.Color.g = 0;
    FillStyles[0].u.Color.b = 0;
    FillStyles[0].u.Color.a = 255;
    /** Now split the path into one or more paths */
    tesselatePath(edges, LineStyles, FillStyles, *shape);

    return shape;
}

}