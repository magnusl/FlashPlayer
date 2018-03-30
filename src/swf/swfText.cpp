#include "swfText.h"
#include "CPlayer.h"
#include "swfDecode.h"
#include "swfTags.h"

namespace swf
{

void CPlayer::DefineText(CTagDecoder & tag)
{
    std::shared_ptr<GlyphText_t> text = std::make_shared<GlyphText_t>();
    uint16_t id = tag.GetU16();
    swf::Decode((avm2::io::CBinarySource &) tag, text->TextBounds);
    swf::Decode((avm2::io::CBinarySource &) tag, text->TextMatrix);
    uint8_t GlyphBits   = tag.GetU8();
    uint8_t AdvanceBits = tag.GetU8();

    uint8_t TextRecordType;
    while(TextRecordType = tag.GetBits(1)) {
        tag.GetBits(3); /** ignore reserved StyleFlagsReserved */

        GlyphTextSegment_t segment;
        segment.HasFont     = tag.GetBits(1);
        segment.HasColor    = tag.GetBits(1);
        segment.HasYOffset  = tag.GetBits(1);
        segment.HasXOffset  = tag.GetBits(1);

        if (segment.HasFont) segment.FontId = tag.GetU16();
        if (segment.HasColor) {
            if (tag.TagCode() == swf::SwfTag_DefineText2) {
                swf::Decode((avm2::io::CBinarySource &)tag, segment.Color.rgba);
            } else {
                swf::Decode((avm2::io::CBinarySource &)tag, segment.Color.rgb);
                segment.Color.rgba.a = 255;
            }
        }
        if (segment.HasYOffset) {
            segment.YOffset = tag.GetS16();
        }
        if (segment.HasXOffset) {
            segment.XOffset = tag.GetS16();
        }
        if (segment.HasFont) {
            segment.TextHeight = tag.GetU16();
        }

        uint8_t GlyphCount         = tag.GetU8();
        segment.Text.resize(GlyphCount);
        for(uint8_t i = 0; i < GlyphCount; ++i) {
            segment.Text[i].GlyphIndex      = tag.GetBits(GlyphBits);
            segment.Text[i].GlyphAdvance    = tag.GetSignedBits(AdvanceBits);
        }
        text->Segments.push_back(segment);
    }
    if (!swfDictionary.addGlyphText(text, id)) {
        throw std::runtime_error("Failed to add glyph text to dictionary.");
    }
}

}