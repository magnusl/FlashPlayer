#ifndef _CDISPLAYOBJECTCONTAINER_H_
#define _CDISPLAYOBJECTCONTAINER_H_

#include "CInteractiveObject.h"
#include <avm2\CClassObject.h>
#include <list>
#include "swfModule.h"

namespace swf
{

/**
 * flash.display.DisplayObjectContainer
 */
class CDisplayObjectContainer : public CInteractiveObject
{
    DECLARE_DERIVED(SWF_DISPLAYOBJECTCONTAINER, CInteractiveObject)

public:
    CDisplayObjectContainer(avm2::CClassObject * cObject, avm2::vtable_t * vt);
    virtual ~CDisplayObjectContainer();

    // adds a child on top of the stacking order
    void addChild(avm2::Handle_t);
    // adds a child at a specific depth
    void addChildAt(avm2::Handle_t, uint16_t);
    // removes a child at a specific depth
    void removeChildAt(uint16_t);
    // removes all the children
    void removeChildren();
    // returns the child at the specified depth
    avm2::Handle_t getChildAt(uint16_t);

    ///////
    // CDisplayObject
    virtual bool hitTestPoint(const gfx::Point_t<float> &, bool pixelPerfect = false);
    virtual void draw(IRenderer &);
    virtual void advanceTimeline(swf::CPlayer &);

    /////
    // GC
    virtual void markReachable();

    /////
    // exported properties
    avm2::Handle_t  getter_numChildren(void);

protected:
    // Use a list for now, optimize later
    std::list<std::pair<uint16_t, CDisplayObject *> > children;

    DECLARE_VTABLE()
};

/**
 * flash.display.DisplayObjectContainer class object.
 */
class CDisplayObjectContainer_Class : public avm2::CClassObject
{
public:
    CDisplayObjectContainer_Class(avm2::CClassObject * cObject, 
        avm2::vtable_t * vt,
        avm2::CVirtualMachine & vm, 
        CClassObject * baseType);

    ~CDisplayObjectContainer_Class();

    /**
     * Constructs an instance of the class.
     *
     * \param [in] arguments    The arguments to supply to the class constructor.
     */
    virtual avm2::Handle_t construct(const std::vector<avm2::Handle_t> & arguments);
    virtual uint32_t getId() const;
};

}

#endif