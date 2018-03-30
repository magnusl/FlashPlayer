#ifndef _CVIRTUALMACHINE_H_
#define _CVIRTUALMACHINE_H_

#include "CScriptObject.h"
#include "Handle.h"
#include "CClassObject.h"
#include "errorcode.h"
#include "CPackage.h"
#include "CStringTable.h"

namespace avm2
{
class CMachineImpl;

namespace abc
{
class CAbcMethodDefinition;
class CAbcClassDefinition;
}

/**
 * Virtual Machine that execute AVM2 bytecode.
 */
class CVirtualMachine
{
public:
    CVirtualMachine();
    virtual ~CVirtualMachine();

    // Registers a package with the virtual machine. The virtual machine
    // will take ownership of the instance and may delete it.
    ErrorCode_t registerPackage(CPackage * package);

    CClassObject * findClass(uint32_t id);
    CClassObject * findClass(const QName & name);

    // executes a abc method
    ErrorCode_t executeMethod(const abc::CAbcMethodDefinition *, Handle_t thisObject,
        Handle_t globalObject, const abc::CAbcClassDefinition *);

    CStringTable & getStringTable() const;

    void collect(avm2::Handle_t);

protected:
    // hide the actual implementation
    CMachineImpl * impl;
};

}

#endif