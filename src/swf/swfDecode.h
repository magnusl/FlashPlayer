#ifndef _DECODE_H_
#define _DECODE_H_

#include <cstdint>
#include <avm2\binaryio.h>
#include "swftypes.h"
#include "swfAction.h"
#include "CTagDecoder.h"
#include <vector>

namespace swf
{

    template<class T>
    void Decode(CTagDecoder &, T &)
    {
        static_assert(false, "Missing type specific implementation of Decode().");
    }
    
    template<class T>
    void Decode(avm2::io::CBinarySource &, T & t)
    {
        static_assert(false, "Missing implementation.");
    }

    template<>
    void Decode<swf::RECT>(avm2::io::CBinarySource &, swf::RECT &);

    template<>
    void Decode<swf::ShapeWithStyle>(CTagDecoder &, swf::ShapeWithStyle &);

    template<>
    void Decode<swf::Shape>(swf::CTagDecoder & io, swf::Shape & shape);

    template<>
    void Decode<swf::Matrix_Fixed2x3>(avm2::io::CBinarySource &, Matrix_Fixed2x3 &);

    template<>
    void Decode<swf::StyleChangedRecord>(CTagDecoder &, swf::StyleChangedRecord & stylechange);

    template<>
    void Decode(avm2::io::CBinarySource &, swf::FileHeader & header);

    template<>
    void Decode(avm2::io::CBinarySource & io, swf::CXFormWithAlpha & cxform);

    template<>
    void Decode<swf::GRADRECORD>(CTagDecoder & io, swf::GRADRECORD & gr);

    template<>
    void Decode<swf::DropShadowFilter>(avm2::io::CBinarySource & io, swf::DropShadowFilter & drop);

    template<>
    void Decode<swf::BlurFilter>(avm2::io::CBinarySource & io, swf::BlurFilter & drop);

    template<>
    void Decode<swf::GlowFilter>(avm2::io::CBinarySource & io, swf::GlowFilter & drop);

    template<>
    void Decode<swf::ColorMatrixFilter>(avm2::io::CBinarySource & io, swf::ColorMatrixFilter & colorMatrix);


    template<>
    void Decode<swf::Filter>(avm2::io::CBinarySource & io, swf::Filter & a_Filter);
    /**
     * Decodes a array of elements.
     */
    template<class T>
    void DecodeArray(CTagDecoder & io, std::vector<T> & arr)
    {
        uint16_t count = io.GetU8();
        count = (count == 0xff) ? io.GetU16() : count;
        //arr.resize(count);
        for(uint16_t i = 0; i < count; i++) {
            T item;
            Decode<T>(io, item);
            arr.push_back(item);
        }
    //  io.AssureAlignment();
        //io.Align();
    }

    void SetBackgroundColor(swf::CTagDecoder & tag, swf::RGB & color);

    template<>
    void Decode<swf::RGBA>(avm2::io::CBinarySource & io, swf::RGBA & rgba);
    
    template<>
    void Decode<swf::RGB>(avm2::io::CBinarySource & io, swf::RGB & rgba);

    template<>
    void Decode<swf::MorphFillStyle>(CTagDecoder & io, swf::MorphFillStyle &);

    template<>
    void Decode<swf::MorphLineStyle>(CTagDecoder & io, swf::MorphLineStyle &);


    void Parse_DefineShape4(swf::CTagDecoder & io, swf::DefinedShape4 & shape);

    void DecodeShape(swf::CTagDecoder & io, swf::DefinedShape & shape);

    void FrameLabel(avm2::io::CBinarySource & source, swf::Frame & frame);
    void PlaceObject(avm2::io::CBinarySource & _source, swf::PlaceObject_t & place);
    void PlaceObject2(avm2::io::CBinarySource & io, swf::PlaceObject2_t & place, avm2::CStringTable & a_StringTable);

    void PlaceObject3(avm2::io::CBinarySource & a_Source, swf::PlaceObject3_t & place, 
        avm2::CStringTable & a_StringTable);

    void RemoveObject(avm2::io::CBinarySource & a_Source, swf::RemoveObject_t & a_Remove);
    
    void RemoveObject2(avm2::io::CBinarySource & a_Source, swf::RemoveObject2_t & a_Remove2);

    float GetEulerAngle(const swf::Matrix_Fixed2x3 & a_Matrix);

    inline float fixed2float(int32_t a_Value)
    {
        return float(a_Value) / 65536.0f;
    }
}

#endif