#ifndef _CPACKAGE_H_
#define _CPACKAGE_H_

//#include "CClassObject.h"
#include <stdint.h>
#include "ObjectDefs.h"
#include "Handle.h"
#include "CClassObject.h"

namespace avm2
{

class CVirtualMachine;

/**
 * A package is used to extend the runtime with additional classes.
 */
class CPackage
{
public:
    CPackage(CVirtualMachine & VirtualMachine) : vm(VirtualMachine)
    {}
    virtual ~CPackage()
    {}

    /**
     * Returns a class object based on the unique id.
     */
    virtual CClassObject * getClassObject(uint32_t id) = 0;

    /**
     * Returns a class object based on it's name
     */
    virtual CClassObject * getClassObject(const QName & name) = 0;

    /**
     * Returns the unique class identifier
     */
    virtual uint32_t getPackageId() const = 0;

    /**
     * Returns a string representing the namespace that the package implements.
     */
    virtual const char * getPackageNamespace() const = 0;

protected:
    CVirtualMachine & vm;
};

}

#endif