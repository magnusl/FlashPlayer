#ifndef _CAVM2PLAYER_H_
#define _CAVM2PLAYER_H_

#include "CPlayer.h"
#include "CDisplayObjectContainer.h"
#include "CDisplayObject.h"
#include "CSprite.h"
#include "CStaticGlyphText.h"
#include <avm2\CABCFile.h>
#include <memory>

namespace swf
{

class CSWFShape;

// ShockWave Flash player using the AVM2 virtual machine
class CAVM2Player : public CPlayer
{
public:
    CAVM2Player(std::shared_ptr<avm2::io::CBinarySource>,
        const swf::FileHeader &);

    virtual ~CAVM2Player();

    virtual avm2::ErrorCode_t Initialize();
    virtual void Draw(IRenderer &);
    virtual avm2::ErrorCode_t Play(size_t frameIndex, float playSpeed=1.0f);
    virtual avm2::ErrorCode_t Pause();
    virtual avm2::ErrorCode_t Advance(float timeDelta);
    virtual float             GetFrameRate() const;
    virtual PlayState_t       GetPlaybackState() const;
    virtual avm2::Handle_t    GetMainTimeline() const;
    virtual void              GC();

protected:

    virtual avm2::CStringTable & getStringTable();
    virtual avm2::ErrorCode_t doPlaceObject2(const swf::Action & a_Action, 
        avm2::Handle_t sprite);
    
    avm2::ErrorCode_t doAddCharacter(const PlaceObject2_t &, CDisplayObjectContainer *);
    avm2::ErrorCode_t doModifyCharacter(const PlaceObject2_t &, CDisplayObjectContainer *);
    avm2::ErrorCode_t doABC(avm2::io::CBinarySource & source);

private:
    avm2::CVirtualMachine       vm;

    /////
    // The parsed ABC files
    std::list<std::shared_ptr<avm2::abc::CABCFile> >    scriptFiles;

    // packages
    avm2::CPackage *            displayPackage;
    avm2::CPackage *            eventPackage;
    // the stage object
    avm2::Handle_t              stageObject;
    // the main timeline object
    avm2::Handle_t              mainTimeline;
    
    std::shared_ptr<MovieClip>  mainTimelineFrames;

    /////
    // The global object
    avm2::Handle_t              globalObject;

    // playback
    PlayState_t                 playState;
    float                       playSpeed;
    float                       updateOffset;
    float                       playbackTime;
};

}

#endif