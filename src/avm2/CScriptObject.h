#ifndef _CSCRIPTOBJECT_H_
#define _CSCRIPTOBJECT_H_

#include "Atom.h"
#include "Names.h"
#include "ObjectDefs.h"
#include "CoreTypes.h"
#include "Handle.h"
#include "vtable.h"
#include "ObjectDefs.h"

#include <assert.h>

namespace avm2
{

namespace abc
{
class CAbcClassDefinition;
}

class CClassObject;

/**
 * The super class of all objects in the ActionScript 3.0 implementation.
 */
class CScriptObject
{
    DECLARE_SUPER(AVM2_OBJECT)

public:

    CScriptObject(CClassObject * cObject, avm2::vtable_t * vt);

    virtual ~CScriptObject();

    // Methods that are exported to the virtual machine.
    bool hasOwnProperty(const Object_MultiName &,
        const abc::CAbcClassDefinition * current = 0);
    bool hasOwnProperty(const QName &,
        const abc::CAbcClassDefinition * current = 0);
    bool hasProperty(const Object_MultiName &,
        const abc::CAbcClassDefinition * current = 0);
    bool hasProperty(const QName &,
        const abc::CAbcClassDefinition * current = 0);

    bool Get(const QName &, Handle_t &,
        const abc::CAbcClassDefinition * current = 0);
    bool Get(const Object_MultiName &, Handle_t &,
        const abc::CAbcClassDefinition * current = 0);
    bool Put(const QName &, Handle_t, bool addNewProperty = true, 
        const abc::CAbcClassDefinition * current = 0);

    bool CanPut(const QName &);
    bool GetSlot(size_t, Handle_t &,
        const abc::CAbcClassDefinition * current = 0);
    bool SetSlot(size_t, Handle_t,
        const abc::CAbcClassDefinition * current = 0);

    avm2::ErrorCode_t CallProperty(CVirtualMachine *,
        const Object_MultiName &, const Handle_t args[], size_t numArgs, Handle_t & returnValue);

    // increments/decrements a counter which keeps the object from being collected.
    void lock();//          { ++lockCount; }
    void unlock();//        { assert(lockCount > 0); --lockCount; }
    inline bool isLocked()      { return lockCount > 0; }
    
    /////
    // GC
    virtual void markReachable();   /**< marks any reachable resources */
    virtual void markUnreachable(); /**< marks the object as unreachable */
    virtual bool isReachable() const;

    /////
    // Property access
    avm2::Handle_t getter_constructor(void);


    void setPrototype(CScriptObject *);

protected:
    CScriptObject(const CScriptObject &);
    CScriptObject & operator=(const CScriptObject &);

protected:
    // Should be overriden by subclasses to provide access to class specific properties.
    virtual std::string toString() const;
    virtual bool _has(const QName &);
    virtual bool _get(const QName &, Handle_t &);

    int getFixedProperty(const QName & name, 
        const abc::CAbcClassDefinition * current);
    int getDynamicProperty(const QName & name);

protected:

    /////
    // Dynamic properties, TODO: replace with foundation::array.
    std::vector<Atom_t>     dynamicValues;
    std::vector<QName>      dynamicNames;

    /////
    // Fixed properties
    const abc::CAbcClassDefinition *    classDefinition;
    std::vector<Atom_t>                 fixedProperties;

    uint16_t    lockCount;      /**< when this counter is greater than zero, the object cannot be collected */
    uint8_t     reachable : 1;  /**< indicates that the object is reachable from the root set */
    /////
    // The constructor object for this instance
    CClassObject *  constructor;
    /////
    // References to the prototype of this instance.
    CScriptObject * prototypeObject;

    avm2::vtable_t * virtualTable;

    /////
    // vtable
    DECLARE_VTABLE()
};

}

#endif