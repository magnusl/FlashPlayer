#ifndef _CCLASSOBJECT_H_
#define _CCLASSOBJECT_H_

#include "Handle.h"
#include "CScriptObject.h"
#include "Names.h"
#include "CStringTable.h"
#include "vtable.h"
#include <vector>

namespace avm2
{

class CVirtualMachine;
class CClassObject_impl;

/**
 * Represents a ActionScript class instance.
 */
class CClassObject : public CScriptObject
{
public:
    /**
     * Constructor.
     */
    CClassObject(CClassObject * cObject, 
        avm2::vtable_t * vt,
        CVirtualMachine &,
        CClassObject * baseType);

    /**
     * Destructor.
     */
    virtual ~CClassObject();

    /**
     * Constructs an instance of the class.
     *
     * \param [in] arguments    The arguments to supply to the class constructor.
     */
    virtual Handle_t construct(const std::vector<Handle_t> & arguments) = 0;
    virtual const QName & getName() const { return name; }
    virtual uint32_t getId() const                                      = 0;

    CScriptObject * getClassProtoype();

    const entry_t * findProperty(CStringTable::index_t) const;

protected:
    // explicit prototype object
    CScriptObject *     classPrototype;
    CVirtualMachine &   vm;
    QName               name;

    DECLARE_VTABLE()
};

/**
 * The class object for "Class".
 */
class CClassObject_Class : public CClassObject
{
public:
    CClassObject_Class(avm2::vtable_t * vt,
        CVirtualMachine &,
        CClassObject * baseType);

    virtual Handle_t construct(const std::vector<Handle_t> & arguments);
    virtual uint32_t getId() const;

    DECLARE_VTABLE()
};

}

#endif