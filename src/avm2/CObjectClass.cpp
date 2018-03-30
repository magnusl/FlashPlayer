#include "CObjectClass.h"

namespace avm2
{
CObject_Class::CObject_Class(avm2::CClassObject * cObject, 
                             avm2::vtable_t * vt,
                             avm2::CVirtualMachine & vm) : 
CClassObject(cObject, vt, vm, nullptr)
{

}

avm2::Handle_t CObject_Class::construct(const std::vector<avm2::Handle_t> & arguments)
{
    return new CScriptObject(this, &CScriptObject::vtable);
}

uint32_t CObject_Class::getId() const
{
    return AVM2_OBJECT;
}

DEFINE_VTABLE(CObject_Class, CClassObject)
END_VTABLE()

}