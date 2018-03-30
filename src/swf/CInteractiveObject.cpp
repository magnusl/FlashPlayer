#include "CInteractiveObject.h"

using namespace avm2;

namespace swf
{

CInteractiveObject_Class::CInteractiveObject_Class(avm2::CClassObject * cObject, 
        avm2::vtable_t * vt,
        avm2::CVirtualMachine & vm, 
        CClassObject * baseType) :
CClassObject(cObject, vt, vm, baseType)
{
    name = QName(vm.getStringTable().set("InteractiveObject"));
}

avm2::Handle_t CInteractiveObject_Class::construct(const std::vector<avm2::Handle_t> & arguments)
{
    return nullptr;
}

uint32_t CInteractiveObject_Class::getId() const
{
    return SWF_INTERACTIVEOBJECT;
}


DEFINE_VTABLE(CInteractiveObject_Class, avm2::CClassObject)
END_VTABLE()

/*****************************************************************************/
/*                                  Instance                                 */
/*****************************************************************************/
DEFINE_VTABLE(CInteractiveObject, CDisplayObject)
END_VTABLE()

} // namespace swf
