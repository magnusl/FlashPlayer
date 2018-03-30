#include "CPlayer.h"
#include "swfTypes.h"
#include "swfDecode.h"
#include "swfTags.h"
#include <avm2\CDeflateSource.h>
#include "CAVM2Player.h"
#include "gfxShape.h"
#include "swfDecode.h"
#include <iostream>
#include "CSprite.h"
#include "CDisplayObjectContainer.h"
#include "swfModule.h"
#include "CSWFShape.h"

using namespace std;
using namespace avm2;

namespace swf
{

CPlayer::CPlayer(std::shared_ptr<avm2::io::CBinarySource> source, const swf::FileHeader & hdr) : 
    swfSource(source),
    swfHdr(hdr)
{
}

CPlayer::~CPlayer()
{
}

void CPlayer::GetStageSize(size_t & width, size_t & height) const
{
    width = swfHdr.FrameSize.Xmax / 20;
    height = swfHdr.FrameSize.Ymax / 20;
}

// Creates a player that can play the SWF file defined by
// the source file.
shared_ptr<CPlayer> CPlayer::CreatePlayer(
        std::shared_ptr<io::CFileSource> _source)
{
    std::shared_ptr<avm2::io::CBinarySource> swfSource;
    swf::FileHeader hdr;
    try {
        // read the signature and version to determine if the file is compressed
        // or not.
        hdr.Signature[2] = _source->GetU8();
        hdr.Signature[1] = _source->GetU8();
        hdr.Signature[0] = _source->GetU8();
        hdr.Version      = _source->GetU8();
        hdr.FileSize     = _source->GetU32();

        if (!memcmp(hdr.Signature, "SWF", 3)) { /**< uncompressed */
            swfSource = _source;
        } else if (!memcmp(hdr.Signature, "SWC", 3)) { /**< zlib compressed */
            swfSource = std::make_shared<avm2::io::CDeflateSource>(_source);
        } else if (!memcmp(hdr.Signature, "SWZ", 3)) { /**< LZMA compressed */
            return nullptr;
        } else {
            return nullptr;
        }

        if (hdr.Version < 8) { /**< unsupported legacy format */
            return nullptr;
        }
            
        // decode the rest of the header
        Decode<swf::FileHeader>(*swfSource, hdr);

        // version 8 or above all starts with a FileAttributes tag.
        swf::CTagDecoder tag(*swfSource);
        if (tag.TagCode() != swf::SwfTag_FileAttributes) {
            return nullptr;
        }
        // read  the flags
        uint8_t reserved        = tag.GetBits(1);
        uint8_t UseDirectBlit   = tag.GetBits(1);
        uint8_t UseGPU          = tag.GetBits(1);
        uint8_t HasMetadata     = tag.GetBits(1);
        uint8_t ActionScript3   = tag.GetBits(1);
        tag.GetBits(24);

        if (ActionScript3) { /** Uses ActionScript 3, use a AVM2Player */
            return std::make_shared<CAVM2Player>(swfSource, hdr);
        } else { /** Does not use ActionScript 3, use a LegacyPlayer */
            return nullptr;
        }
    } catch(std::runtime_error &) {
        return nullptr;
    }
}

/**
 * Parses a definition tag and updates the dictionary.
 */
void CPlayer::ParseDefinition(uint16_t tagCode, CTagDecoder & tag)
{
    uint16_t id;
    switch(tagCode) 
    {
    case SwfTag_DefineShape:
    case SwfTag_DefineShape2:
    case SwfTag_DefineShape3:
    case SwfTag_DefineShape4:
        {
            // Parse the shape and add it to the dictionary
            if (std::shared_ptr<swf::gfx::Shape_t> ptr = DefineShape(tag, id)) {
                swfDictionary.addShape(ptr, id);
            }
            break;
        }
    case SwfTag_DefineSprite:
        {
            // Parse the sprite/movieclip and add it to the dictionary
            if (std::shared_ptr<swf::MovieClip> ptr = DefineSprite(tag, id)) {
                swfDictionary.addMovieClip(ptr, id);
            }
            break;
        }
    case SwfTag_DefineText:
        {
            DefineText(tag);
            break;
        }
    case SwfTag_DefineFont3:
        {
            DefineFont3(tag);
            break;
        }
    }
}

/**
 * Parses a shape definition tag.
 */
std::shared_ptr<swf::gfx::Shape_t> 
    CPlayer::DefineShape(CTagDecoder & tag, uint16_t & a_Id)
{
    swf::DefinedShape4 swfShape;
    Parse_DefineShape4(tag, swfShape);
    a_Id = swfShape.ShapeId;
    return gfx::CreateShape(swfShape);
}

std::shared_ptr<swf::MovieClip> 
    CPlayer::DefineSprite(CTagDecoder & tag, uint16_t & id)
{
    id = tag.GetU16();
    uint16_t frameCount = tag.GetU16();

    std::shared_ptr<swf::MovieClip> mc = std::make_shared<swf::MovieClip>();
    
    mc->frames.resize(frameCount);
    size_t currentFrame     = 0;
    while(currentFrame < frameCount) 
    {
        Frame * pCurrentFrame = &mc->frames[currentFrame];
        CTagDecoder controlTag((avm2::io::CBinarySource &) tag);
        switch(controlTag.TagCode())
        {
        case SwfTag_ShowFrame:  ++currentFrame; break;
        case SwfTag_PlaceObject:
        case SwfTag_PlaceObject2:
        case SwfTag_PlaceObject3:
        case SwfTag_RemoveObject:
        case SwfTag_RemoveObject2:
        case SwfTag_StartSound:
        case SwfTag_StartSound2:
        case SwfTag_FrameLabel:
        case SwfTag_SoundStreamHead:
        case SwfTag_SoundStreamHead2:
        case SwfTag_SoundStreamBlock:
            ParseAction(controlTag.TagCode(), controlTag, *pCurrentFrame);
            break;
        case SwfTag_End:
            return mc;
        default:
            throw std::runtime_error("Unsupported tag in sprite definition.");
        }
    }
    return mc;
}

/**
 * Parses a frame action.
 */
void CPlayer::ParseAction(uint16_t tagCode, avm2::io::CBinarySource & source, swf::Frame & frame)
{
    size_t actionIndex;
    Action & action = getNextAction(actionIndex);

    switch(tagCode) 
    {
    case swf::SwfTag_FrameLabel:
        FrameLabel(source, frame); break;
    case swf::SwfTag_PlaceObject:
        action.type = ePlaceObject;
        PlaceObject(source, action.u.Place); 
        break;
    case swf::SwfTag_PlaceObject2:
        action.type = ePlaceObject2;
        PlaceObject2(source, action.u.Place2, getStringTable()); 
        break;
    case swf::SwfTag_PlaceObject3:
        action.type = ePlaceObject3;
        PlaceObject3(source, action.u.Place3, getStringTable());
        break;
    case swf::SwfTag_RemoveObject:
        action.type = eRemoveObject;
        RemoveObject(source, action.u.Remove);
        break;
    case swf::SwfTag_RemoveObject2:
        action.type = eRemoveObject2;
        RemoveObject2(source, action.u.Remove2);
        break;
    case swf::SwfTag_SetBackgroundColor:
        action.type = eSetBackground;
        break;
    default:
        throw std::runtime_error("Unknown action.");
    }
    frame.ActionIndicies.push_back(actionIndex);
}

/**
 * Executes the actions of a frame.
 */
avm2::ErrorCode_t CPlayer::ExecuteFrame(const swf::Frame & a_Frame, avm2::Handle_t sprite)
{
    for(size_t i = 0, n = a_Frame.ActionIndicies.size(); i < n; ++i)
    {
        const swf::Action & action = swfActions[a_Frame.ActionIndicies[i]];
        switch(action.type)
        {
        case ePlaceObject2: doPlaceObject2(action, sprite); break;
        default: break;
        }
    }
    return avm2::Err_Ok;
}

Action & CPlayer::getNextAction(size_t & a_Index)
{
    Action action;
    a_Index = swfActions.size();
    swfActions.push_back(action);
    return swfActions[a_Index];
}

}