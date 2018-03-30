#ifndef _CTOPLEVELPACKAGE_H_
#define _CTOPLEVELPACKAGE_H_

#include "CPackage.h"
#include "CStringTable.h"

namespace avm2
{

/**
 * Package for the top-level objects.
 */
class CTopLevelPackage : public CPackage
{
public:
    CTopLevelPackage(CVirtualMachine & VirtualMachine);
    virtual ~CTopLevelPackage();

    virtual CClassObject * getClassObject(uint32_t id);
    virtual CClassObject * getClassObject(const QName & name);
    virtual uint32_t getPackageId() const;
    virtual const char * getPackageNamespace() const;

protected:
    std::vector<CClassObject *> classObjects;
    std::vector<CStringTable::index_t> classNames;
};

}

#endif