#include "CEventPackage.h"
#include <avm2\CRuntimeClass.h>
#include <avm2\CVirtualMachine.h>
#include "swfModule.h"
#include <assert.h>
#include <cstring>
#include "CEventDispatcher.h"

using namespace avm2;
using namespace std;

namespace swf
{

static const char * eventClasses[] =
{
    "EventDispatcher"
};

CEventPackage::CEventPackage(avm2::CVirtualMachine & VirtualMachine) : 
    CPackage(VirtualMachine)
{
    classObjects.resize(SWF_EventMax);
    classNames.resize(SWF_EventMax);

    CStringTable & st = VirtualMachine.getStringTable();
    for(size_t i = 0; i < SWF_EventMax; ++i) {
        classNames[i] = st.set(eventClasses[i]);
    }
}


CEventPackage::~CEventPackage()
{
    for(size_t i = 0; i < SWF_EventMax; ++i) {
        delete classObjects[i];
    }
}

/**
 * Returns a class object.
 */
avm2::CClassObject * CEventPackage::getClassObject(uint32_t id)
{
    switch(id)
    {
    case SWF_EVENTDISPATCHER:
        if (!classObjects[SWF_EventDispatcher]) {
            classObjects[SWF_EventDispatcher] = new (std::nothrow) CEventDispatcher_Class(
                vm.findClass(AVM2_CLASSOBJECT), 
                &CEventDispatcher_Class::vtable, 
                vm, 
                vm.findClass(AVM2_OBJECT));
        }
        return classObjects[SWF_EventDispatcher];
    default:
        return nullptr;
    }
}

CClassObject * CEventPackage::getClassObject(const QName & name)
{
    for(size_t i = 0; i < SWF_EventMax; ++i)
    {
        if (name == classNames[i]) {
            return getClassObject(SWF_EVENTDISPATCHER + i);
        }
    }
    return nullptr;
}

/**
 * Returns the unique class identifier
 */
uint32_t CEventPackage::getPackageId() const
{
    return SWF_EVENTS_PACKAGE_ID >> 24;
}

/**
 * Returns a string representing the namespace that the package implements.
 */
const char * CEventPackage::getPackageNamespace() const
{
    return "flash.events";
}

}