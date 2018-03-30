#ifndef _CEVENTDISPATCHER_H_
#define _CEVENTDISPATCHER_H_

/**
 * \file    CEventDispatcher.h
 * \brief   Defines the CEventDispatcher class that implements the 
 *          functionality of the ActionScript 3 flash.event.EventDispatcher class.
 * \author  Magnus Leksell
 */

#include <avm2\CScriptObject.h>
#include <avm2\CStringTable.h>
#include <avm2\CClassObject.h>
#include "swfModule.h"

namespace swf
{

/** 
 * Defines the flash.event.EventDispatcher class.
 */
class CEventDispatcher : public avm2::CScriptObject
{
    DECLARE_DERIVED(SWF_EVENTDISPATCHER, avm2::CScriptObject)

public:
    CEventDispatcher(avm2::CClassObject * cObject, avm2::vtable_t * vt);

    // Registers a event listener
    avm2::ErrorCode_t addEventListener(avm2::CStringTable::index_t type, 
        avm2::Handle_t listener,
        bool useCapture = false);

    // returns true if the dispatcher has a matching listener
    bool hasEventListener(avm2::CStringTable::index_t type);

    /////
    // GC
    virtual void markReachable();   /**< marks any reachable resources */

protected:

    DECLARE_VTABLE()
};

class CEventDispatcher_Class : public avm2::CClassObject
{
public:
    CEventDispatcher_Class(avm2::CClassObject * cObject, 
        avm2::vtable_t * vt,
        avm2::CVirtualMachine &,
        avm2::CClassObject * baseType);

    virtual avm2::Handle_t construct(const std::vector<avm2::Handle_t> & arguments);
    virtual uint32_t getId() const;

    DECLARE_VTABLE()
};

}

#endif