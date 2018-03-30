
#include "CScriptObject.h"
#include "CoreTypes.h"
#include "CVirtualMachine.h"

namespace avm2
{
#if 0
CEmbeddedClass::CEmbeddedClass(CVirtualMachine & virtualMachine, 
                               CClassObject * constructorObject,
                               CClassObject * baseType,
                               CPackage & packageRef,
                               uint32_t classId) :
CClassObject(virtualMachine, constructorObject),
package(packageRef)
{
    /////
    // When we create the initial 'Object' class instance we cannot
    // call searchSymbol, since the 'Object' instance hasn't been constructed yet.
    // In that case the constructor class for the newly created 'Object' instance
    // is the "this" pointer.
    CClassObject * cObj = (classId == AVM2_OBJECT) ? this : virtualMachine.findClass(AVM2_OBJECT);
    /////
    // Create the class prototype.
    prototype = new (std::nothrow) CScriptObject(this);
    if (baseType) {
        /////
        // Link the prototype object to the prototype of the superclass,
        // and thus form a prototype chain.
        prototype->setPrototype(baseType->getClassProtoype());
    }
}
#endif

}