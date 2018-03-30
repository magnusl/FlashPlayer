#ifndef _CRUNTIMECLASS_H_
#define _CRUNTIMECLASS_H_

#include "CClassObject.h"
#include "CPackage.h"

namespace avm2
{

#if 0
class CEmbeddedClass : public CClassObject
{
public:
    CEmbeddedClass(CVirtualMachine &, 
        CClassObject * constructorObject,
        CClassObject * baseType,
        CPackage & packageRef,
        uint32_t classId);

    virtual Handle_t construct();

protected:
    CPackage & package;
};

#endif
}

#endif