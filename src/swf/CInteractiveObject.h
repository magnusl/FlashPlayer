#ifndef _CINTERACTIVEOBJECT_H_
#define _CINTERACTIVEOBJECT_H_

#include "CDisplayObject.h"
#include "swfModule.h"

namespace swf
{

/**
 * flash.display.InteractiveObject
 */
class CInteractiveObject : public CDisplayObject
{
    DECLARE_DERIVED(SWF_INTERACTIVEOBJECT, CDisplayObject)
public:
    CInteractiveObject(avm2::CClassObject * cObject, avm2::vtable_t * vt) :
        CDisplayObject(cObject, vt)
    {
    }

    DECLARE_VTABLE()
};

/**
 * flash.display.DisplayObject class object.
 */
class CInteractiveObject_Class : public avm2::CClassObject
{
public:
    CInteractiveObject_Class(avm2::CClassObject * cObject, 
        avm2::vtable_t * vt,
        avm2::CVirtualMachine & vm, 
        CClassObject * baseType);
    /**
     * Constructs an instance of the class.
     *
     * \param [in] arguments    The arguments to supply to the class constructor.
     */
    virtual avm2::Handle_t construct(const std::vector<avm2::Handle_t> & arguments);
    virtual uint32_t getId() const;

    DECLARE_VTABLE();
};

}

#endif