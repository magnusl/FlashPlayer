#ifndef _CMOVIECLIP_H_
#define _CMOVIECLIP_H_

#include "CSprite.h"
#include "swfModule.h"
#include <avm2\vtable.h>
#include <avm2\CClassObject.h>
#include <avm2\CVirtualMachine.h>

namespace swf
{

/**
 * Represents a movie clip, flash.display.MovieClip
 */
class CMovieClip : public CSprite
{
    DECLARE_DERIVED(SWF_MOVIECLIP, CSprite)

public:
    CMovieClip(avm2::CClassObject * cObject, avm2::vtable_t * vt);
    virtual ~CMovieClip();

    virtual void advanceTimeline(swf::CPlayer &);
    virtual bool isDone() const;
    virtual void resetTimeline();

    /////
    // exported properties
    avm2::Handle_t  getter_currentFrame(void);
    avm2::Handle_t  getter_currentFrameLabel(void);
    avm2::Handle_t  getter_currentLabel(void);
    avm2::Handle_t  getter_enabled(void);
    avm2::Handle_t  getter_framesLoaded(void);
    avm2::Handle_t  getter_totalFrames(void);

protected:
    int32_t currentFrame;

    DECLARE_VTABLE()
};

class CMovieClip_Class : public avm2::CClassObject
{
public:
    CMovieClip_Class(avm2::CClassObject * cObject, 
        avm2::vtable_t * vt,
        avm2::CVirtualMachine & vm,
        avm2::CClassObject * baseType);

    virtual avm2::Handle_t construct(const std::vector<avm2::Handle_t> & arguments);
    virtual uint32_t getId() const;
};

}

#endif