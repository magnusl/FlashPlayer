#include "CClassObject.h"
#include "CVirtualMachine.h"
#include <map>

using namespace std;

namespace avm2
{

/*****************************************************************************/
/*                                  Class                                    */
/*****************************************************************************/

CClassObject_Class::CClassObject_Class(avm2::vtable_t * vt,
        CVirtualMachine & vm,
        CClassObject * baseType) : CClassObject(nullptr, vt, vm, baseType)
{

}

Handle_t CClassObject_Class::construct(const std::vector<Handle_t> & arguments)
{
    return nullptr;
}

uint32_t CClassObject_Class::getId() const
{
    return AVM2_CLASSOBJECT;
}

DEFINE_VTABLE(CClassObject_Class, CClassObject)
END_VTABLE()

/*****************************************************************************/
/*                                  Instance                                 */
/*****************************************************************************/

CClassObject::CClassObject(CClassObject * cObject, 
                           avm2::vtable_t * vt,
                           CVirtualMachine & virtualmachine,
                           CClassObject * baseType) :
    CScriptObject(cObject, vt), 
    vm(virtualmachine)
{
    /////
    // Create a new object that will act as the prototype of this class
    classPrototype = new (std::nothrow) CScriptObject(virtualmachine.findClass(AVM2_OBJECT),
        &CScriptObject::vtable);
    /////
    //  Now link it with the base class prototype object and thus form a prototype chain
    if (baseType) {
        classPrototype->setPrototype(baseType->getClassProtoype());
    }
}
    
CClassObject::~CClassObject()
{

}

CScriptObject * CClassObject::getClassProtoype()
{
    return classPrototype;
}

const entry_t * CClassObject::findProperty(CStringTable::index_t) const
{
    return nullptr;
}

DEFINE_VTABLE(CClassObject, CScriptObject)
END_VTABLE()

}