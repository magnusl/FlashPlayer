#ifndef _CPLAYER_H_
#define _CPLAYER_H_

#include <avm2\binaryio.h>
#include <memory>
#include <avm2\errorcode.h>
#include <avm2\Handle.h>
#include <avm2\CVirtualMachine.h>

#include "CTagDecoder.h"
#include "gfxShape.h"
#include "swfAction.h"
#include "CDictionary.h"
#include "IRenderer.h"

namespace swf
{

typedef enum {
    PLAYBACK_STOPPED,
    PLAYBACK_PLAYING,
    PLAYBACK_PAUSED
} PlayState_t;

/**
 * Super class for the different SWF players.
 */
class CPlayer
{
public:
    // Creates a player that can play the SWF file defined by the source file.
    static std::shared_ptr<CPlayer> CreatePlayer(
        std::shared_ptr<avm2::io::CFileSource> _source);

    // initializes the common player properties
    CPlayer(std::shared_ptr<avm2::io::CBinarySource> source,const swf::FileHeader & hdr);
    // destructor, performs the required cleanup
    virtual ~CPlayer();
    // initialize the player
    virtual avm2::ErrorCode_t Initialize() = 0;
    // returns the size of the stage, in pixels
    void GetStageSize(size_t & width, size_t & height) const;
    // draws the scene
    virtual void Draw(IRenderer &) = 0;
    // starts playing at the specified frame with the specified speed
    virtual avm2::ErrorCode_t Play(size_t frameIndex = 0, float playSpeed=1.0f) = 0;
    // pauses the playback
    virtual avm2::ErrorCode_t Pause() = 0;
    // advances the playback, should be called reapetadly with the elapsed time
    virtual avm2::ErrorCode_t Advance(float timeDelta) = 0;
    // returns the frame rate
    virtual float GetFrameRate() const = 0;
    // returns the current playback status
    virtual PlayState_t GetPlaybackState() const = 0;
    // executes a single frame
    virtual avm2::ErrorCode_t ExecuteFrame(const swf::Frame &, avm2::Handle_t);
    // returns the main timeline
    virtual avm2::Handle_t GetMainTimeline() const = 0;
    virtual void GC() = 0;

protected:
    CPlayer(const CPlayer &);
    CPlayer(std::shared_ptr<avm2::io::CBinarySource>);

    CPlayer & operator =(const CPlayer &);

    virtual avm2::CStringTable & getStringTable() = 0;
    std::shared_ptr<swf::gfx::Shape_t>  DefineShape(CTagDecoder & tag, uint16_t & a_Id);
    std::shared_ptr<swf::MovieClip>     DefineSprite(CTagDecoder & tag, uint16_t & a_Id);
    void DefineFont3(CTagDecoder & tag);
    void DefineText(CTagDecoder & tag);

    void ParseAction(uint16_t tagCode, avm2::io::CBinarySource &, swf::Frame & frame);
    void ParseDefinition(uint16_t, CTagDecoder &);

    virtual avm2::ErrorCode_t doPlaceObject2(const swf::Action & a_Action, 
        avm2::Handle_t sprite) = 0;

    Action & getNextAction(size_t & a_Index);

protected:
    swf::FileHeader                             swfHdr;
    std::shared_ptr<avm2::io::CBinarySource>    swfSource;
    CDictionary                                 swfDictionary;
    std::vector<swf::Action>                    swfActions;
};

}

#endif