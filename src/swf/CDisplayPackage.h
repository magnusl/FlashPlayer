#ifndef _CDISPLAYPACKAGE_H_
#define _CDISPLAYPACKAGE_H_

#include <avm2\CPackage.h>
#include <avm2\CClassObject.h>
#include <avm2\CStringTable.h>
#include "swfModule.h"

namespace swf
{

/**
 * Defines the flash.display package and initializes all the classes
 * in that package.
 */
class CDisplayPackage : public avm2::CPackage
{
public:
    CDisplayPackage(avm2::CVirtualMachine & VirtualMachine);
    virtual ~CDisplayPackage();

    /**
     * Returns a class object.
     */
    virtual avm2::CClassObject * getClassObject(uint32_t id);
    virtual avm2::CClassObject * getClassObject(const avm2::QName & name);

    /**
     * Returns the unique class identifier
     */
    virtual uint32_t getPackageId() const;

    /**
     * Returns a string representing the namespace that the package implements.
     */
    virtual const char * getPackageNamespace() const;

protected:
    std::vector<avm2::CClassObject *>   classObjects;
    std::vector<avm2::CStringTable::index_t> classNames;
};

}

#endif