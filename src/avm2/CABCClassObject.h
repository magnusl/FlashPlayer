#ifndef _CABCCLASSOBJECT_H_
#define _CABCCLASSOBJECT_H_

#include "CAbcClassDefinition.h"
#include "CoreTypes.h"
#include "CClassObject.h"

namespace avm2
{

class CABCClassObject : public CClassObject
{
    DECLARE_DERIVED(AVM2_ABCCLASSOBJECT, CClassObject)

public:

protected:
    const abc::CAbcClassDefinition * classDef;
};

}

#endif