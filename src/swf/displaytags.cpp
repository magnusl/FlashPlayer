#include <avm2\binaryio.h>
#include "swfTags.h"
#include "swfTypes.h"
#include "swfDecode.h"
#include "swfAction.h"

namespace swf
{
/**
 * Decodes a PlaceObject tag.
 */
void PlaceObject(avm2::io::CBinarySource & _source, swf::PlaceObject_t & place)
{
    place.CharacterId   = _source.GetU16();
    place.Depth         = _source.GetU16();

    Decode<Matrix_Fixed2x3>(_source, place.TransformationMatrix);
}

/**
 * Decodes a PlaceObject2 tag.
 */
void PlaceObject2(avm2::io::CBinarySource & source, swf::PlaceObject2_t & place, 
    avm2::CStringTable & stringTable)
{
    source.AssureAlignment();
    place.PlaceFlagHasClipActions       = source.GetBits(1);
    place.PlaceFlagHasClipDepth         = source.GetBits(1);
    place.PlaceFlagHasName              = source.GetBits(1);
    place.PlaceFlagHasRatio             = source.GetBits(1);
    place.PlaceFlagHasColorTransform    = source.GetBits(1);
    place.PlaceFlagHasMatrix            = source.GetBits(1);
    place.PlaceFlagHasCharacter         = source.GetBits(1);
    place.PlaceFlagMove                 = source.GetBits(1);

    source.AssureAlignment();

    place.Depth                         = source.GetU16();
    if(place.PlaceFlagHasCharacter) {
        place.CharacterId = source.GetU16();
    }
    if(place.PlaceFlagHasMatrix) {
        swf::Decode<Matrix_Fixed2x3>(source, place.TransformationMatrix);
    }
    if(place.PlaceFlagHasColorTransform) {
        swf::Decode(source, place.cxform);
    }
    place.Ratio     = place.PlaceFlagHasRatio ? source.GetU16() : 0;
    if(place.PlaceFlagHasRatio) {
        //place.Ratio = a_Source.GetU16();
    }
    if(place.PlaceFlagHasName) {
        std::string name = source.GetString();
        place.Name = stringTable.set(name);
    } else {
        place.Name = 0;
    }
    if (place.PlaceFlagHasClipDepth) {
        place.ClipDepth = source.GetU16();
    } else {
        place.ClipDepth = 0;
    }
}

/**
 * Decodes a PlaceObject3 tag.
 */
void PlaceObject3(avm2::io::CBinarySource & source, swf::PlaceObject3_t & place, 
    avm2::CStringTable & a_StringTable)
{
    place.PlaceFlagHasClipActions       = source.GetBits(1);
    place.PlaceFlagHasClipDepth         = source.GetBits(1);
    place.PlaceFlagHasName              = source.GetBits(1);
    place.PlaceFlagHasRatio             = source.GetBits(1);
    place.PlaceFlagHasColorTransform    = source.GetBits(1);
    place.PlaceFlagHasMatrix            = source.GetBits(1);
    place.PlaceFlagHasCharacter         = source.GetBits(1);
    place.PlaceFlagMove                 = source.GetBits(1);
        
    place.Reserved                      = source.GetBits(1);
    place.PlaceFlagOpaqueBackground     = source.GetBits(1);
    place.PlaceFlagHasVisible           = source.GetBits(1);
    place.PlaceFlagHasImage             = source.GetBits(1);
    place.PlaceFlagHasClassName         = source.GetBits(1);
    place.PlaceFlagHasCacheAsBitmap     = source.GetBits(1);
    place.PlaceFlagHasBlendMode         = source.GetBits(1);
    place.PlaceFlagHasFilterList        = source.GetBits(1);
        
    place.Depth                         = source.GetU16();

    if (place.PlaceFlagHasClassName ||
        (place.PlaceFlagHasImage && place.PlaceFlagHasCharacter))
    {
        std::string className = source.GetString();
        place.ClassName = a_StringTable.set(className);
    }

    if(place.PlaceFlagHasCharacter) {
        place.CharacterId = source.GetU16();
    }

    if(place.PlaceFlagHasMatrix) {
        swf::Decode<Matrix_Fixed2x3>(source, place.TransformationMatrix);
    }
    if(place.PlaceFlagHasColorTransform) {
        swf::Decode(source, place.cxform);
    }

    place.Ratio     = place.PlaceFlagHasRatio ? source.GetU16() : 0;
    if(place.PlaceFlagHasRatio) {
        place.Ratio = source.GetU16();
    }
    if(place.PlaceFlagHasName) {
        std::string name = source.GetString();
        place.Name = a_StringTable.set(name);
    } else {
        place.Name = 0;
    }
    place.ClipDepth = place.PlaceFlagHasClipDepth ? source.GetU16() : 0;

    if (place.PlaceFlagHasFilterList) {
        uint8_t numFilter = source.GetU8();
        place.NumFilters = (numFilter > 8) ? 8 : numFilter; /**< we only care for the first 8 */
        for(size_t i = 0; i < numFilter; ++i)
        {
            if (i < 8) {
                Decode(source, place.Filters[i]);
            } else { /**< ignore remaining */
                swf::Filter filter;
                Decode(source, filter);
            }
        }
    }

    if (place.PlaceFlagHasBlendMode) {
        place.BlendMode = source.GetU8();
    }
    if (place.PlaceFlagHasCacheAsBitmap) {
        place.BitmapCache = source.GetU8();
    }
    if (place.PlaceFlagHasVisible) {
        place.Visible = source.GetU8();
    }
    if (place.PlaceFlagOpaqueBackground) {
        Decode((avm2::io::CBinarySource &) source, place.BackgroundColor);
    }
    if (place.PlaceFlagHasClipActions) {
        throw std::runtime_error("TODO: CLIPACTIONS.");
    }
}

void RemoveObject(avm2::io::CBinarySource & source, swf::RemoveObject_t & remove)
{
    remove.CharacterId  = source.GetU16();
    remove.Depth        = source.GetU16();
}

void RemoveObject2(avm2::io::CBinarySource & source, swf::RemoveObject2_t & remove2)
{
    remove2.Depth           = source.GetU16();
}

void FrameLabel(avm2::io::CBinarySource & source, swf::Frame & frame)
{
    frame.Label = source.GetString();
}

}