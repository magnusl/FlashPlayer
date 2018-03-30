#ifndef _CSPRITE_H_
#define _CSPRITE_H_

#include "CDisplayObjectContainer.h"
#include "swfModule.h"
#include <avm2\CClassObject.h>

namespace swf
{

/**
 * Implements the functionality of flash.display.Sprite
 */
class CSprite : public CDisplayObjectContainer
{
    DECLARE_DERIVED(SWF_SPRITE, CDisplayObjectContainer)
public:
    CSprite(avm2::CClassObject * cObject, avm2::vtable_t * vt);

    void setFrames(std::shared_ptr<MovieClip> _frames) { frames = _frames; }
    virtual bool isDone() const { return false; }
    virtual void resetTimeline() {}

    /////
    // exported properties
    avm2::Handle_t  getter_buttonMode(void);
    avm2::Handle_t  getter_dropTarget(void);
    avm2::Handle_t  getter_graphics(void);
    avm2::Handle_t  getter_hitArea(void);

public:
    std::shared_ptr<MovieClip> frames;

    DECLARE_VTABLE()
};

class CSprite_Class : public avm2::CClassObject
{
public:
    CSprite_Class(avm2::CClassObject * cObject, 
        avm2::vtable_t * vt,
        avm2::CVirtualMachine & vm, 
        CClassObject * baseType);
    virtual avm2::Handle_t construct(const std::vector<avm2::Handle_t> & arguments);
    virtual uint32_t getId() const;
};

}

#endif