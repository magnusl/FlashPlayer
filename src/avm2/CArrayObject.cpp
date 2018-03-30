#include "CArrayObject.h"

namespace avm2
{

/*****************************************************************************/
/*                                      Class                                */
/*****************************************************************************/

CArray_Class::CArray_Class(CClassObject * cObject, 
        avm2::vtable_t * vt,
        CVirtualMachine & vm,
        CClassObject * baseType) : 
CClassObject(cObject, vt, vm, baseType)
{
}

Handle_t CArray_Class::construct(const std::vector<Handle_t> & arguments)
{
    return nullptr;
}

uint32_t CArray_Class::getId() const
{
    return AVM2_ARRAY;
}

DEFINE_VTABLE(CArray_Class, CClassObject)
END_VTABLE()

/**
 * Define the vtable that actually exports the methods to the script.
 */
DEFINE_VTABLE(CArrayObject, CScriptObject)
    EXPORT_PROPERTY("length", &CArrayObject::getter_length, nullptr)
END_VTABLE()



//////
// Property access
avm2::Handle_t CArrayObject::getter_length()
{
    return Handle_t();
}

}