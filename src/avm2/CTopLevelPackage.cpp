#include "CTopLevelPackage.h"
#include "CoreTypes.h"
#include "CObjectClass.h"
#include "CVirtualMachine.h"
#include "CClassObject.h"
#include "CObjectClass.h"

namespace avm2
{

static const char * topLevelClasses[] =
{
    "Object",
    "Array",
    "String",
    "Class",
    "Function",
    "__abc__Class",
};

CTopLevelPackage::CTopLevelPackage(CVirtualMachine & VirtualMachine) :
    CPackage(VirtualMachine)
{
    classObjects.resize(AVM2_MaxClass);
    classNames.resize(AVM2_MaxClass);

    CStringTable & st = VirtualMachine.getStringTable();
    for(size_t i = 0; i < AVM2_MaxClass; ++i)
    {
        classNames[i] = st.set(topLevelClasses[i]);
    }
}

CTopLevelPackage::~CTopLevelPackage()
{
    for(size_t i = 0; i < AVM2_MaxClass; ++i) {
        delete classObjects[i];
    }
}

CClassObject * CTopLevelPackage::getClassObject(uint32_t id)
{
    switch(id)
    {
    case AVM2_CLASSOBJECT:
        if (!classObjects[AVM2_ClassObject]) 
        {
            classObjects[AVM2_ClassObject] = new (std::nothrow) 
                CClassObject_Class(&CClassObject_Class::vtable, vm, nullptr);
        }
        return classObjects[AVM2_ClassObject];
    case AVM2_OBJECT:
        if (!classObjects[AVM2_Object])
        {
            classObjects[AVM2_Object] = new (std::nothrow)
                CObject_Class(vm.findClass(AVM2_CLASSOBJECT), &CObject_Class::vtable, vm);
        }
        return classObjects[AVM2_Object];
    default:
        return nullptr;
    }
}

CClassObject * CTopLevelPackage::getClassObject(const QName & name)
{
    for(size_t i = 0; i < AVM2_MaxClass; ++i) {
        if (name.Name == classNames[i]) {
            return getClassObject(AVM2_OBJECT + i);
        }
    }
    return nullptr;
}

uint32_t CTopLevelPackage::getPackageId() const
{
    return AVM2_MODULE_ID;
}

const char * CTopLevelPackage::getPackageNamespace() const
{
    return "";
}

}