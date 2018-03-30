#include "CVirtualMachine.h"
#include "CClassObject.h"
#include "CScriptObject.h"
#include <iostream>
#include <assert.h>
#include <list>
#include <map>
#include "CInterpreter.h"
#include "CABCClassObject.h"
#include <memory>
#include "CRuntimeClass.h"
#include "CoreTypes.h"
#include "CTopLevelPackage.h"

using namespace std;

namespace avm2
{

// The actual implementation of the Virtual Machine.
class CMachineImpl
{
public:
    CMachineImpl();
    ~CMachineImpl();

    CStringTable                        stringTable;
    std::map<PackageID_t, CPackage *>   packages;
    std::list<CScriptObject *>          objects;
    std::shared_ptr<CInterpreter>       interpreter;
    std::map<uint32_t, CClassObject *>  idMap;
    std::map<uint32_t, CClassObject *>  nameMap;
    avm2::CTopLevelPackage *            toplevelPackage;
};

CMachineImpl::CMachineImpl()
{
    stringTable.set("");
}

CMachineImpl::~CMachineImpl()
{
}

// Initializes the virtual machine instance.
CVirtualMachine::CVirtualMachine()
{
    impl = new (std::nothrow) CMachineImpl;
    impl->interpreter       = std::make_shared<CInterpreter>(impl->stringTable, this);
    impl->toplevelPackage   = new (std::nothrow) CTopLevelPackage(*this);
    registerPackage(impl->toplevelPackage);
}

// Performs the required cleanup.
CVirtualMachine::~CVirtualMachine()
{
    delete impl->toplevelPackage;
    delete impl;
}

CClassObject * CVirtualMachine::findClass(uint32_t id)
{
    /////
    // check the cache first
    map<uint32_t, CClassObject *>::iterator it = impl->idMap.find(id);
    if (it != impl->idMap.end()) {
        return it->second;
    }
    uint32_t pid = (id >> 24);
    /////
    // Check packages
    for(map<PackageID_t, CPackage *>::iterator it = impl->packages.begin(); 
        it != impl->packages.end(); 
        it++)
    {
        CClassObject * obj = it->second->getClassObject(id);
        if (obj) {
            impl->idMap[id]                         = obj;
            impl->nameMap[obj->getName().toU32()]   = obj;
        }
        return obj;
    }
    /////
    // did not find the class.
    return nullptr;
}

CClassObject * CVirtualMachine::findClass(const QName & name)
{
    uint32_t key = name.toU32();

    map<uint32_t, CClassObject *>::iterator it = impl->nameMap.find(key);
    if (it != impl->nameMap.end()) {
        return it->second;
    }
    /////
    // Check packages
    for(map<PackageID_t, CPackage *>::iterator it = impl->packages.begin(); 
        it != impl->packages.end(); 
        it++)
    {
        CClassObject * obj = it->second->getClassObject(name);
        if (obj) {
            impl->idMap[obj->getId()]               = obj;
            impl->nameMap[obj->getName().toU32()]   = obj;
        }
        return obj;
    }
}

/**
 * Implementation of a simple Mark and Sweep GC. All objects are first marked as unreachable.
 * Then the root set is searched and marks all reachable objects as reachable.
 * The objects that are still marked as unreachable is then removed, unless that are 
 * locked in memory (someone has a handle to it).
 */
void CVirtualMachine::collect(avm2::Handle_t root)
{
    CScriptObject * rootObj = toObject(root);
    size_t collected = 0;
    if (impl->objects.size() > 1024) {
        // first mark all objects as unreachable
        for(std::list<CScriptObject *>::iterator it = impl->objects.begin();it != impl->objects.end();it++)
        {
            (*it)->markUnreachable();
        }
        // mark reachable objects
        rootObj->markReachable();
        // delete unreachable objects
        std::list<CScriptObject *> reachable;
        for(std::list<CScriptObject *>::iterator it = impl->objects.begin();it != impl->objects.end(); it++)
        {
            if ((*it)->isReachable() || (*it)->isLocked()) {
                reachable.push_back(*it);
            } else {
                ++collected;
                delete *it;
            }
        }
        impl->objects.clear();
        impl->objects = reachable;
    }
}

ErrorCode_t CVirtualMachine::registerPackage(CPackage * _package)
{
    int id = _package->getPackageId();
    impl->packages[id] = _package;
    return Err_Ok;
}

CStringTable & CVirtualMachine::getStringTable() const
{
    return impl->stringTable;   
}

/////
// Executes a method defined in a ABC file.
ErrorCode_t CVirtualMachine::executeMethod(const abc::CAbcMethodDefinition * method, 
                                           Handle_t thisObject,
                                           Handle_t globalObject, 
                                           const abc::CAbcClassDefinition * classDefinition)
{
    if (impl && impl->interpreter) {
        /////
        // Execute the method using the interpreter.
        return impl->interpreter->execute(method, thisObject,
            globalObject, 0U, classDefinition);
    } else {
        return Err_NotImplemented;
    }
}

}