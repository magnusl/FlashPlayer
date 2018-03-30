#include "swfdecode.h"
#include "swftypes.h"
#include "swftags.h"
#include <iostream>

#include <assert.h>

namespace swf
{
    inline uint32_t Fixed16_16(int16_t i, int16_t f)
    {
        return (i << 16) | f;
    }

    void DecodeStyleChange(swf::CTagDecoder & io, swf::StyleChangedRecord & stylechange, swf::ShapeWithStyle & a_Shape);
    void DecodeStyleChange(swf::CTagDecoder & io, swf::StyleChangedRecord & stylechange, swf::Shape & a_Shape);

    /*************************************************************************/
    /*                                  Basic types                          */
    /*************************************************************************/
    template<>
    void Decode(avm2::io::CBinarySource & io, swf::RGBA & rgba)
    {
        rgba.r = io.GetU8();
        rgba.g = io.GetU8();
        rgba.b = io.GetU8();
        rgba.a = io.GetU8();
    }

    template<>
    void Decode(avm2::io::CBinarySource & io, swf::RGB & rgb)
    {
        rgb.r = io.GetU8();
        rgb.g = io.GetU8();
        rgb.b = io.GetU8();
    }

    /**
     * Decodes the SWF file header from the io source.
     */
    template<>
    void Decode(avm2::io::CBinarySource & io, swf::FileHeader & header)
    {
        /** Decode signature, SWF or SWC */
    /*  header.Signature[2] = io.GetU8();
        header.Signature[1] = io.GetU8();
        header.Signature[0] = io.GetU8();
        header.Version      = io.GetU8();
        header.FileSize     = io.GetU32();*/

        Decode<swf::RECT>(io, header.FrameSize);

        header.FrameRate    = io.GetU16();
        header.FrameCount   = io.GetU16();
    }

    /**
     * Decodes a SWF RECT record from the io source. The coordinates are specified as twips, and are
     * are stored in a variable length format.
     *
     * \param       io      The io source to decode the swf::RECT record from.
     * \param       rect    The decoded swf::RECT.
     */
    template<>
    void Decode(avm2::io::CBinarySource & io, swf::RECT & rect)
    {
        io.AssureAlignment();
        uint8_t NBits = static_cast<uint8_t>(io.GetBits(5));
        /** Sign-extend and read the coordinates */
        rect.Xmin = io.GetSignedBits(NBits);
        rect.Xmax = io.GetSignedBits(NBits);
        rect.Ymin = io.GetSignedBits(NBits);
        rect.Ymax = io.GetSignedBits(NBits);
        /** align the io to byte boundry */
        io.Align();
    }

    template<>
    void Decode(avm2::io::CBinarySource & io, Matrix_Fixed2x3 & matrix)
    {
        io.AssureAlignment();
        /** Scaling */
        matrix.HasScale = io.GetBits(1);
        if (matrix.HasScale) {
            uint8_t NScaleBits = io.GetBits(5);
            matrix.ScaleX = io.GetFixed16(NScaleBits);
            matrix.ScaleY = io.GetFixed16(NScaleBits);
        } else {
            matrix.ScaleX = Fixed16_16(1, 0);
            matrix.ScaleY = Fixed16_16(1, 0);
        }
        /** rotate */
        matrix.HasRotate = io.GetBits(1);
        if (matrix.HasRotate) {
            uint8_t NRotateBits = io.GetBits(5);
            matrix.RotateSkew0 = io.GetFixed16(NRotateBits);
            matrix.RotateSkew1 = io.GetFixed16(NRotateBits);
        } else {
            matrix.RotateSkew0 = matrix.RotateSkew1 = Fixed16_16(0, 0);
        }
        /** translate */
        uint8_t NTranslateBits = io.GetBits(5);
        matrix.TranslateX = io.GetSignedBits(NTranslateBits);
        matrix.TranslateY = io.GetSignedBits(NTranslateBits);
        io.Align();
    }

    /*************************************************************************/
    /*                                  Shapes                               */
    /*************************************************************************/
    template<>
    void Decode(CTagDecoder & io, swf::StraightEdgeRecord & straight)
    {
        size_t NumBits = io.GetBits(4);
        uint8_t GeneralLineFlag = io.GetBits(1);
        uint8_t VertLineFlag = (GeneralLineFlag ? 0 : io.GetBits(1));
        straight.DeltaX = (GeneralLineFlag || (!VertLineFlag)) ? io.GetSignedBits(NumBits+2) : 0;
        straight.DeltaY = (GeneralLineFlag || (VertLineFlag)) ? io.GetSignedBits(NumBits+2) : 0;
    }

    /**
     * Decodes a Curved edge record which defines a Quadratic Bezier curve.
     */
    template<>
    void Decode<swf::CurvedEdgeRecord>(CTagDecoder & io, swf::CurvedEdgeRecord & curved)
    {
        uint8_t NumBits         = io.GetBits(4) + 2;
        curved.ControlDeltaX    = io.GetSignedBits(NumBits);
        curved.ControlDeltaY    = io.GetSignedBits(NumBits);
        curved.AnchorDeltaX     = io.GetSignedBits(NumBits);
        curved.AnchorDeltaY     = io.GetSignedBits(NumBits);
    }

    /**
     * Decodes a shape record.
     */
    void DecodeShapeRecord(CTagDecoder & io, swf::ShapeRecord & a_Record, swf::ShapeWithStyle & a_Shape)
    {
        if (io.GetBits(1)) {        /** edge record */
            if (io.GetBits(1)) {
                a_Record.m_RecordType = swf::ShapeRecord::STRAIGHT_EDGE_RECORD;
                Decode<swf::StraightEdgeRecord>(io, a_Record.u.StraightEdge);
            } else {
                a_Record.m_RecordType = swf::ShapeRecord::CURVED_EDGE_RECORD;
                Decode<swf::CurvedEdgeRecord>(io, a_Record.u.CurvedEdge);
            }
        } else {

            a_Record.u.StyleChanged.cflags          = 0;
            a_Record.u.StyleChanged.StateNewStyles  = io.GetBits(1);
            a_Record.u.StyleChanged.StateLineStyle      = io.GetBits(1);
            a_Record.u.StyleChanged.StateFillStyle1     = io.GetBits(1);
            a_Record.u.StyleChanged.StateFillStyle0     = io.GetBits(1);
            a_Record.u.StyleChanged.StateMoveTo     = io.GetBits(1);

            if (a_Record.u.StyleChanged.cflags == 0) {
                /** this is a end record */
                a_Record.m_RecordType = swf::ShapeRecord::END_SHAPE_RECORD;
            } else {
                /** we need to supply this information when parsing the style change record */
                a_Record.u.StyleChanged.NumFillBits     = a_Record.NumFillBits;
                a_Record.u.StyleChanged.NumLineBits     = a_Record.NumLineBits;
                a_Record.u.StyleChanged.LineStyleOffset = a_Record.LineStyleOffset;
                a_Record.u.StyleChanged.FillStyleOffset = a_Record.FillStyleOffset;

                DecodeStyleChange(io, a_Record.u.StyleChanged, a_Shape);
                if (a_Record.u.StyleChanged.StateNewStyles) {
                    a_Record.NumFillBits = a_Record.u.StyleChanged.NumFillBits;
                    a_Record.NumLineBits = a_Record.u.StyleChanged.NumLineBits;
                }
                a_Record.m_RecordType = swf::ShapeRecord::STYLE_CHANGE_RECORD;
                a_Record.FillStyleOffset = a_Record.u.StyleChanged.FillStyleOffset;
                a_Record.LineStyleOffset = a_Record.u.StyleChanged.LineStyleOffset;
            }
        }
    }

    /**
     * Decodes a shape record.
     */
    void DecodeShapeRecord(CTagDecoder & io, swf::ShapeRecord & a_Record, swf::Shape & a_Shape)
    {
        if (io.GetBits(1)) {        /** edge record */
            if (io.GetBits(1)) {
                a_Record.m_RecordType = swf::ShapeRecord::STRAIGHT_EDGE_RECORD;
                Decode<swf::StraightEdgeRecord>(io, a_Record.u.StraightEdge);
            } else {
                a_Record.m_RecordType = swf::ShapeRecord::CURVED_EDGE_RECORD;
                Decode<swf::CurvedEdgeRecord>(io, a_Record.u.CurvedEdge);
            }
        } else {
            a_Record.u.StyleChanged.cflags          = 0;
            a_Record.u.StyleChanged.StateNewStyles  = io.GetBits(1);
            a_Record.u.StyleChanged.StateLineStyle  = io.GetBits(1);
            a_Record.u.StyleChanged.StateFillStyle1 = io.GetBits(1);
            a_Record.u.StyleChanged.StateFillStyle0 = io.GetBits(1);
            a_Record.u.StyleChanged.StateMoveTo     = io.GetBits(1);

            if (a_Record.u.StyleChanged.cflags == 0) {
                /** this is a end record */
                a_Record.m_RecordType = swf::ShapeRecord::END_SHAPE_RECORD;
            } else {
                /** we need to supply this information when parsing the style change record */
                a_Record.u.StyleChanged.NumFillBits     = a_Record.NumFillBits;
                a_Record.u.StyleChanged.NumLineBits     = a_Record.NumLineBits;
                a_Record.u.StyleChanged.LineStyleOffset = a_Record.LineStyleOffset;
                a_Record.u.StyleChanged.FillStyleOffset = a_Record.FillStyleOffset;

                DecodeStyleChange(io, a_Record.u.StyleChanged, a_Shape);
                a_Record.m_RecordType = swf::ShapeRecord::STYLE_CHANGE_RECORD;
            }
        }
    }

    template<>
    void Decode<swf::GRADRECORD>(CTagDecoder & io, swf::GRADRECORD & gr)
    {
        gr.ratio = io.GetU8();
        uint16_t tagCode = io.TagCode();
        if (tagCode == SwfTag_DefineShape || tagCode == SwfTag_DefineShape2) {
            Decode((avm2::io::CBinarySource &) io, gr.color.rgb);
            gr.color.rgba.a = 255;
        } else {
            Decode((avm2::io::CBinarySource &) io, gr.color.rgba);
        }
    }

    template<>
    void Decode<swf::Filter>(avm2::io::CBinarySource & io, swf::Filter & a_Filter)
    {
        a_Filter.Type = io.GetU8();
        switch(a_Filter.Type) 
        {
        case 0: Decode(io, a_Filter.dropShadow); break;
        case 1: Decode(io, a_Filter.blur); break;
        case 2: Decode(io, a_Filter.glow); break;
        case 6: Decode(io, a_Filter.colorMatrix); break;
        default: throw std::runtime_error("Filter type not implemented.");
        }
    }

    template<>
    void Decode<swf::DropShadowFilter>(avm2::io::CBinarySource & io, swf::DropShadowFilter & drop)
    {
        Decode(io, drop.DropShadowColor);
        drop.BlurX              = io.GetFixed16(32);
        drop.BlurY              = io.GetFixed16(32);
        drop.Angle              = io.GetFixed16(32);
        drop.Distance           = io.GetFixed16(32);
        drop.Strength           = io.GetFixed16(16); // TODO: FIX, should be FIXED8
        drop.InnerShadow        = io.GetBits(1);
        drop.Knockout           = io.GetBits(1);
        drop.CompositeSource    = io.GetBits(1);
        drop.Passes             = io.GetBits(5);
    }

    template<>
    void Decode<swf::BlurFilter>(avm2::io::CBinarySource & io, swf::BlurFilter & blur)
    {
        blur.BlurX              = io.GetFixed16(32);
        blur.BlurY              = io.GetFixed16(32);
        blur.Passes             = io.GetBits(5);
        io.GetBits(3);
    }

    template<>
    void Decode<swf::GlowFilter>(avm2::io::CBinarySource & io, swf::GlowFilter & glow)
    {
        Decode(io, glow.glowColor);
        glow.BlurX = io.GetFixed16(32);
        glow.BlurY = io.GetFixed16(32);
        glow.Strength = io.GetFixed8();
        glow.InnerGlow = io.GetBits(1);
        glow.KnockOut = io.GetBits(1);
        glow.CompositeSource = io.GetBits(1);
        glow.Passes = io.GetBits(3);
    }

    template<>
    void Decode<swf::ColorMatrixFilter>(avm2::io::CBinarySource & io, swf::ColorMatrixFilter & colorMatrix)
    {
        for(size_t i = 0; i < 20; ++i)
        {
            colorMatrix.Matrix[i] = io.GetFloat();
        }
    }

    template<>
    void Decode<swf::FillStyle>(CTagDecoder & io, swf::FillStyle & fs)
    {
        uint16_t tagCode = io.TagCode();
        fs.FillStyleType = io.GetU8();
        switch(fs.FillStyleType) {
        case 0x00: /** solid fill */
            if ((io.TagCode() == swf::SwfTag_DefineShape) || (io.TagCode() == swf::SwfTag_DefineShape2)) { 
                Decode<swf::RGB>((avm2::io::CBinarySource &)io, fs.color.rgb);
                fs.color.rgba.a = 255;
            } else {
                Decode<swf::RGBA>((avm2::io::CBinarySource &)io, fs.color.rgba);
            }
            fs.FillType = swf::FillStyle::eSOLID_RGBA;
            break;
        case 0x10: /** linear gradient fill */
        case 0x12:
            {
                swf::Matrix_Fixed2x3 mat;
                Decode((avm2::io::CBinarySource &)io, mat);
                uint8_t SpreadMode = io.GetBits(2);
                uint8_t interpolationMode = io.GetBits(2);
                if ((SpreadMode != 0) || (interpolationMode != 0)) {
                    if ((tagCode == SwfTag_DefineShape) || 
                        (tagCode == SwfTag_DefineShape2) || 
                        (tagCode == SwfTag_DefineShape3)) 
                    {
                        throw std::runtime_error("The SpreadMode/InterpolationMode parameter of the GRADIENT record must be 0 for this tag");
                    }
                }
                uint8_t NumGradients = io.GetBits(4);
                if (NumGradients > 8) {
                    if ((tagCode == SwfTag_DefineShape) || 
                        (tagCode == SwfTag_DefineShape2) || 
                        (tagCode == SwfTag_DefineShape3)) 
                    {
                        throw std::runtime_error("The NumGradients field cannot exceed 8 for this tag");
                    }
                }
                if (fs.FillStyleType == 0x10) {
                    fs.FillType = FillStyle::eLINEAR_GRADIENT;
                } else {
                    fs.FillType = FillStyle::eRADIAL_GRADIENT;
                }
                fs.LinearGradient.numControlPoints  = NumGradients;
                for(size_t i = 0; i < NumGradients; ++i) {
                    /** decode the gradient control points */
                    Decode(io, fs.LinearGradient.controlPoint[i]);
                }
                break;
            }
        case 0x13: /**< focal radial fill */
            {
                swf::Matrix_Fixed2x3 mat;
                Decode((avm2::io::CBinarySource &)io, mat);
                uint8_t SpreadMode = io.GetBits(2);
                uint8_t interpolationMode = io.GetBits(2);
                if ((SpreadMode != 0) || (interpolationMode != 0)) {
                    if ((tagCode == SwfTag_DefineShape) || 
                        (tagCode == SwfTag_DefineShape2) || 
                        (tagCode == SwfTag_DefineShape3)) 
                    {
                        throw std::runtime_error("The SpreadMode/InterpolationMode parameter of the GRADIENT record must be 0 for this tag");
                    }
                }
                uint8_t NumGradients = io.GetBits(4);
                if (NumGradients > 8) {
                    if ((tagCode == SwfTag_DefineShape) || 
                        (tagCode == SwfTag_DefineShape2) || 
                        (tagCode == SwfTag_DefineShape3)) 
                    {
                        throw std::runtime_error("The NumGradients field cannot exceed 8 for this tag");
                    }
                }
                fs.FillType                         = FillStyle::eFOCAL_GRADIENT;
                fs.FocalGradient.numControlPoints  = NumGradients;
                for(size_t i = 0; i < NumGradients; ++i) {
                    /** decode the gradient control points */
                    Decode(io, fs.FocalGradient.controlPoint[i]);
                }
                fs.FocalGradient.focalPoint = io.GetFixed8();
                break;
            }
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
            if (fs.FillStyleType == 0x40)       fs.FillType = FillStyle::eREPEAT_BITMAP;
            else if(fs.FillStyleType == 0x41)   fs.FillType = FillStyle::eCLIPPED_BITMAP;
            else if(fs.FillStyleType == 0x42)   fs.FillType = FillStyle::eNON_SMOOTH_REPEAT_BITMAP;
            else if(fs.FillStyleType == 0x43)   fs.FillType = FillStyle::eNON_SMOOTH_CLIPPED_BITMAP;

            fs.Bitmap.bitmapId = io.GetU16();
            Decode((avm2::io::CBinarySource&) io, fs.Bitmap.matrix);
            break;
        default:
            assert(false);
        }
    }

    /**
     * Solid fill color with opacity information.
     */
    template<>
    void Decode<LineStyle>(CTagDecoder & io, swf::LineStyle & ls)
    {
        uint16_t tagCode = io.TagCode();
        if ((tagCode == SwfTag_DefineShape) || (tagCode == SwfTag_DefineShape2) || (tagCode == SwfTag_DefineShape3)) {
            ls.type = LineStyle::LINESTYLE1;
            ls.LineStyle1.width = io.GetU16();
            if ((tagCode == SwfTag_DefineShape) || (tagCode == SwfTag_DefineShape2)) {
                Decode<swf::RGB>((avm2::io::CBinarySource &)io, ls.LineStyle1.color.rgb);
                ls.LineStyle1.color.rgba.a = 255;
            } else {
                Decode<swf::RGBA>((avm2::io::CBinarySource &)io, ls.LineStyle1.color.rgba);
            }
        } else {
            ls.type = LineStyle::LINESTYLE2;
            ls.Linestyle2.Width         = io.GetU16();
            ls.Linestyle2.StartCapStyle = io.GetBits(2);
            ls.Linestyle2.JoinStyle     = io.GetBits(2);
            ls.Linestyle2.HasFillFlag   = io.GetBits(1);
            ls.Linestyle2.NoHScaleFlag  = io.GetBits(1);
            ls.Linestyle2.NoVScaleFlag  = io.GetBits(1);
            ls.Linestyle2.PixelHintingFlag = io.GetBits(1);
            io.GetBits(5); /**< 5 reserved bits */
            ls.Linestyle2.NoClose       = io.GetBits(1);
            ls.Linestyle2.EndCapStyle   = io.GetBits(2);
            ls.Linestyle2.MiterLimitFactor = (ls.Linestyle2.JoinStyle == 2) ? io.GetU16() : 0;
            if(!ls.Linestyle2.HasFillFlag) {
                Decode((avm2::io::CBinarySource &)io, ls.Linestyle2.Color.rgba);
            } else {
                Decode(io, ls.Linestyle2.FillType);
            }
        }
    }

    /**
     * Decodes a shape definition used by DefineShape, DefineShape2 and DefineShape3.
     */
    template<>
    void Decode<swf::ShapeWithStyle>(swf::CTagDecoder & io, swf::ShapeWithStyle & shape)
    {
        DecodeArray<swf::FillStyle>(io, shape.FillStyles);
        DecodeArray<swf::LineStyle>(io, shape.LineStyles);
        io.AssureAlignment();
        
        /** read shape records until a end shape record is reached */
        swf::ShapeRecord record;
        record.NumFillBits = io.GetBits(4);
        record.NumLineBits = io.GetBits(4);
        record.FillStyleOffset = 0;
        record.LineStyleOffset = 0;

        record.u.StyleChanged.LineStyle     = -1;
        record.u.StyleChanged.FillStyle0    = -1;
        record.u.StyleChanged.FillStyle1    = -1;
        
        do {
            DecodeShapeRecord(io, record, shape);
            shape.Shapes.push_back(record);
        } while(record.m_RecordType != ShapeRecord::END_SHAPE_RECORD);
        io.Align();
    }

    template<>
    void Decode<swf::Shape>(swf::CTagDecoder & io, swf::Shape & shape)
    {
        /** read shape records until a end shape record is reached */
        swf::ShapeRecord record;
        record.NumFillBits = io.GetBits(4);
        record.NumLineBits = io.GetBits(4);
        record.FillStyleOffset = 0;
        record.LineStyleOffset = 0;

        do {
            DecodeShapeRecord(io, record, shape);
            shape.Shapes.push_back(record);
        } while(record.m_RecordType != swf::ShapeRecord::END_SHAPE_RECORD);
    }

    void DecodeStyleChange(swf::CTagDecoder & io, 
        swf::StyleChangedRecord & stylechange,
        swf::ShapeWithStyle & a_Shape)
    {
        if (stylechange.StateMoveTo) {
            uint8_t MoveBits        = io.GetBits(5);
            stylechange.MoveDeltaX  = io.GetSignedBits(MoveBits);
            stylechange.MoveDeltaY  = io.GetSignedBits(MoveBits);
        } else {
            stylechange.MoveDeltaX  = 0;
            stylechange.MoveDeltaY  = 0;
        }
        if ((stylechange.StateFillStyle0) && (stylechange.NumFillBits > 0)) {
            stylechange.FillStyle0 = io.GetBits(stylechange.NumFillBits) + stylechange.FillStyleOffset;
        }
        if ((stylechange.StateFillStyle1) && (stylechange.NumFillBits > 0)) {
            stylechange.FillStyle1 = io.GetBits(stylechange.NumFillBits) + stylechange.FillStyleOffset;
        }
        if ((stylechange.StateLineStyle) && (stylechange.NumLineBits > 0)) {
            stylechange.LineStyle = io.GetBits(stylechange.NumLineBits) + stylechange.LineStyleOffset;
        }
        if (stylechange.StateNewStyles) {
            if((io.TagCode() != SwfTag_DefineShape2) &&
                (io.TagCode() != SwfTag_DefineShape3) &&
                (io.TagCode() != SwfTag_DefineShape4)) 
            {
                return;
            }
            /** read the new fill and linestyles */
            size_t numFillStyles = a_Shape.FillStyles.size();
            size_t numLineStyles = a_Shape.LineStyles.size();
            DecodeArray<swf::FillStyle>(io, a_Shape.FillStyles);
            DecodeArray<swf::LineStyle>(io, a_Shape.LineStyles);

            stylechange.NumFillBits = io.GetBits(4);
            stylechange.NumLineBits = io.GetBits(4);

            stylechange.LineStyleOffset = numLineStyles;
            stylechange.FillStyleOffset = numFillStyles;

            stylechange.LineStyle   = -1;
            stylechange.FillStyle0  = -1;
            stylechange.FillStyle1  = -1;
        }
    }

    void DecodeStyleChange(swf::CTagDecoder & io, swf::StyleChangedRecord & stylechange, swf::Shape & a_Shape)
    {
        if (stylechange.StateMoveTo) {
            uint8_t MoveBits        = io.GetBits(5);
            stylechange.MoveDeltaX  = io.GetSignedBits(MoveBits);
            stylechange.MoveDeltaY  = io.GetSignedBits(MoveBits);
        } else {
            stylechange.MoveDeltaX  = 0;
            stylechange.MoveDeltaY  = 0;
        }
        if (stylechange.StateFillStyle0) {
            stylechange.FillStyle0 = io.GetBits(stylechange.NumFillBits) + stylechange.FillStyleOffset;
        }
        if (stylechange.StateFillStyle1) {
            stylechange.FillStyle1 = io.GetBits(stylechange.NumFillBits) + stylechange.FillStyleOffset;
        }
        if (stylechange.StateLineStyle) {
            stylechange.LineStyle = io.GetBits(stylechange.NumLineBits) + stylechange.LineStyleOffset;
        }
    }

    template<>
    void Decode<swf::CXFormWithAlpha>(avm2::io::CBinarySource & io, swf::CXFormWithAlpha & cxform)
    {
        io.Align();
        cxform.HasAddTerms  = io.GetBits(1);
        cxform.HasMultTerms = io.GetBits(1);

        uint8_t Nbits = io.GetBits(4);
        if (cxform.HasMultTerms) {
            cxform.RedMultTerm = io.GetSignedBits(Nbits);
            cxform.GreenMultTerm = io.GetSignedBits(Nbits);
            cxform.BlueMultTerm = io.GetSignedBits(Nbits);
            cxform.AlphaMultTerm = io.GetSignedBits(Nbits);
        }
        if (cxform.HasAddTerms) {
            cxform.RedAddTerm = io.GetSignedBits(Nbits);
            cxform.GreenAddTerm = io.GetSignedBits(Nbits);
            cxform.BlueAddTerm = io.GetSignedBits(Nbits);
            cxform.AlphaAddTerm = io.GetSignedBits(Nbits);
        }
    }

    template<>
    void Decode<swf::MorphFillStyle>(CTagDecoder & io, 
        swf::MorphFillStyle & a_Style)
    {
        switch(io.GetU8())
        {
        case 0x00:
            Decode((avm2::io::CBinarySource&)io, a_Style.solid.StartColor);
            Decode((avm2::io::CBinarySource&)io, a_Style.solid.EndColor);
            break;
        default:
            throw std::runtime_error("TODO: This morph style has not been implemented.");
        }
    }

    template<>
    void Decode<swf::MorphLineStyle>(CTagDecoder & io, swf::MorphLineStyle & a_Style)
    {
        if (io.TagCode() == SwfTag_DefineMorphShape2) {
            a_Style.IsVerision2 = 1;
            a_Style.version2.StartWidth     = io.GetU16();
            a_Style.version2.EndWidth       = io.GetU16();
            a_Style.version2.StartCapStyle  = io.GetBits(2);
            a_Style.version2.JoinStyle      = io.GetBits(2);
            a_Style.version2.HasFillFlag    = io.GetBits(1);
            a_Style.version2.NoHScaleFlag   = io.GetBits(1);
            a_Style.version2.NoVScaleFlag   = io.GetBits(1);
            a_Style.version2.PixelHintingFlag   = io.GetBits(1);
            io.GetBits(5); /**< reserved */

            a_Style.version2.NoClose        = io.GetBits(1);
            a_Style.version2.EndCapStyle    = io.GetBits(2);
            if (a_Style.version2.JoinStyle == 2) {
                a_Style.version2.MiterLimitFactor = io.GetU16();
            }
            if (a_Style.version2.HasFillFlag == 0) {
                Decode((avm2::io::CBinarySource &)io, a_Style.version2.StartColor);
                Decode((avm2::io::CBinarySource &)io, a_Style.version2.EndColor);
            } else {
                throw std::runtime_error("Support for filled morph lines not implemented.");
            }
        } else {
            a_Style.IsVerision2 = 0;
            a_Style.version1.StartWidth = io.GetU16();
            a_Style.version1.EndWidth = io.GetU16();
            Decode((avm2::io::CBinarySource &)io, a_Style.version1.StartColor);
            Decode((avm2::io::CBinarySource &)io, a_Style.version1.EndColor);
        }
    }
}