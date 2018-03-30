#include "CTagDecoder.h"
#include "swfTypes.h"
#include "swfDecode.h"
#include "swfTags.h"
#include "CUnimplementedException.h"

namespace swf
{

/**
 * Parses a DefineShape tag (Tag type = 2)
 */
void Parse_DefineShape(swf::CTagDecoder & io, swf::DefinedShape & shape)
{
    shape.ShapeId = io.GetU16();
    Decode<swf::RECT>((avm2::io::CBinarySource &)io, shape.ShapeBounds);
    Decode<swf::ShapeWithStyle>(io, shape.Shapes);
}

/**
 * Parses a DefineShape4 tag.
 */
void Parse_DefineShape4(swf::CTagDecoder & io, swf::DefinedShape4 & shape)
{
    shape.ShapeId = io.GetU16();
    Decode<swf::RECT>((avm2::io::CBinarySource &)io, shape.ShapeBounds);
    if (io.TagCode() == SwfTag_DefineShape4) {
        Decode<swf::RECT>((avm2::io::CBinarySource &)io, shape.EdgesBounds);
        io.GetBits(5);
        shape.UsesFillWindingRule   = io.GetBits(1);
        shape.UsesNonScalingStrokes = io.GetBits(1);
        shape.UsesScalingStrokes    = io.GetBits(1);
    }
    io.AssureAlignment();
    Decode<swf::ShapeWithStyle>(io, shape.Shapes);
}

/**
 * Pareses a button record.
 */
void Parse_Record(swf::CTagDecoder & io, Buttonrecord_t & a_Record, uint8_t a_Flags)
{
    a_Record.ButtonHasBlendMode     = (a_Flags >> 5) & 0x01;
    a_Record.ButtonHasFilterList    = (a_Flags >> 4) & 0x01;
    a_Record.ButtonStateHitTest     = (a_Flags >> 3) & 0x01;
    a_Record.ButtonStateDown        = (a_Flags >> 2) & 0x01;
    a_Record.ButtonStateOver        = (a_Flags >> 1) & 0x01;
    a_Record.ButtonStateUp          = (a_Flags >> 0) & 0x01;

    a_Record.CharacterID    = io.GetU16();
    a_Record.PlaceDepth     = io.GetU16();
    Decode((avm2::io::CBinarySource &) io, a_Record.Matrix);

    if (io.TagCode() == SwfTag_DefineButton2) {
        Decode((avm2::io::CBinarySource &)io, a_Record.CxForm);
        if (a_Record.ButtonHasFilterList) {
            throw swf::CUnimplementedException("Filter list parsing not implemented.");
        }
        if (a_Record.ButtonHasBlendMode) {
            a_Record.BlendMode = io.GetU8();
        }
    }
}

/**
 * Parses a DefineButton tag.
 */
void Parse_DefineButton(Button_t & a_Button, swf::CTagDecoder & io, uint16_t & a_Id)
{
    a_Id                    = io.GetU16();
    uint8_t flags           = io.GetU8();
    a_Button.TrackAsMenu    = false;
    do {
        /** parse button record */
        Buttonrecord_t record;
        Parse_Record(io, record, flags);
        a_Button.Records.push_back(record);
        flags = io.GetU8();
    } while(flags);

    uint8_t ActionEndFlag = io.GetU8();
    if (ActionEndFlag) {
        throw swf::CUnimplementedException("Actions are not supported in DefineButton.");
    }
}

/**
 * Parses a DefineButton2 tag.
 */
void Parse_DefineButton2(Button_t & a_Button, swf::CTagDecoder & io, uint16_t & a_Id)
{
    a_Id    = io.GetU16();
    io.GetBits(7);  /**< reserved */
    a_Button.TrackAsMenu = io.GetBits(1) ? true : false;
    uint16_t ActionOffset = io.GetU16();
    if (ActionOffset) {
        throw std::runtime_error("Actions are not supported in DefineButton2.");
    }
    uint8_t flags       = io.GetU8();
    do {
        /** parse button record */
        Buttonrecord_t record;
        Parse_Record(io, record, flags);
        a_Button.Records.push_back(record);
        flags = io.GetU8();
    } while(flags);
}

} // namespace swf
