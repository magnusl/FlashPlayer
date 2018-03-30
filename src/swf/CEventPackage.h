/**
 * \file    CEventPackage.h
 * \brief   Defines the package that implements the flash.event ActionScript package.
 * \author  Magnus Leksell
 */

#ifndef _CEVENTPACKAGE_H_
#define _CEVENTPACKAGE_H_

#include <avm2\CPackage.h>
#include <avm2\CClassObject.h>
#include "swfModule.h"

namespace swf
{

/**
 * Defines the flash.events package and initializes all the classes
 * in that package.
 */
class CEventPackage : public avm2::CPackage
{
public:
    CEventPackage(avm2::CVirtualMachine & VirtualMachine);
    virtual ~CEventPackage();

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
    std::vector<avm2::CClassObject *>           classObjects;
    std::vector<avm2::CStringTable::index_t>    classNames;
};

}

#endif