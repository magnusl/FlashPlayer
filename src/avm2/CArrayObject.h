#include "CScriptObject.h"
#include "CoreTypes.h"
#include "CClassObject.h"

namespace avm2
{

/** 
 * ActionScript 3.0 Array.
 */
class CArrayObject : public CScriptObject
{
    DECLARE_DERIVED(AVM2_ARRAY, CScriptObject)

protected:
    CArrayObject(const CArrayObject &);
    CArrayObject & operator=(const CArrayObject &);

public:
    CArrayObject(CClassObject * _constructorObject);

    /////
    // Property access
    avm2::Handle_t getter_length();

protected:
    virtual std::string toString() const;

    /////
    // vtable
    DECLARE_VTABLE()
};

class CArray_Class : public CClassObject
{
public:
    CArray_Class(CClassObject * cObject, 
        avm2::vtable_t * vt,
        CVirtualMachine &,
        CClassObject * baseType);

    virtual Handle_t construct(const std::vector<Handle_t> & arguments);
    virtual uint32_t getId() const;

    DECLARE_VTABLE()
};

}