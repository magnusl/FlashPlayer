#include "CScriptObject.h"
#include "CVirtualMachine.h"
#include "vtable.h"
#include "CAbcClassDefinition.h"

#include <assert.h>

using namespace std;

namespace avm2
{

CScriptObject::CScriptObject(CClassObject * cObject, avm2::vtable_t * vt) :
    constructor(cObject),
    lockCount(0),
    reachable(0),
    classDefinition(0),
    prototypeObject(0),
    virtualTable(vt)
{

}

CScriptObject::~CScriptObject()
{
    // if we try to delete a object while still holding a hard
    // lock to it then something is wrong.
    assert(lockCount == 0);
}

void CScriptObject::lock()
{
    ++lockCount;
}

void CScriptObject::unlock()
{
    assert(lockCount > 0);
    if (lockCount) {
        --lockCount;
    }
}

void CScriptObject::markReachable()
{
    reachable = 1;
    // mark all dynamic properties as reachable
    for(size_t i = 0, num = dynamicValues.size(); i < num; ++i) {
        if (CScriptObject * obj = toObject(dynamicValues[i])) {
            obj->markReachable();
        }
    }
    if (prototypeObject)    prototypeObject->markReachable();
    if (constructor)        constructor->markReachable();
}

void CScriptObject::setPrototype(CScriptObject * prototype)
{
    prototypeObject = prototype;
}

void CScriptObject::markUnreachable()
{
    reachable = 0;
}

bool CScriptObject::isReachable() const
{
    return reachable ? true : false;
}

int CScriptObject::getFixedProperty(const QName & name, 
                                    const abc::CAbcClassDefinition * current)
{
    if (classDefinition) {
        if (current) {

        }
        for(size_t i = 0, num = classDefinition->traitNames.size();
            i < num;
            ++i) 
        {
            if (classDefinition->traitNames[i] == name) {
                return (int) i;
            }
        }
    }
    return -1;
}

int CScriptObject::getDynamicProperty(const QName & name)
{
    for(size_t i = 0, num = dynamicNames.size(); i < num; ++i) {
        if (name == dynamicNames[i]) {
            return (int) i;
        }
    }
    return -1;
}

/**
 * Implements [[Get]]
 */
bool CScriptObject::Get(const QName & name, 
                        Handle_t & value,
                        const abc::CAbcClassDefinition * current)
{
    /////
    // Check if any runtime property matches.
    if (constructor) {
        if (const entry_t * entry = constructor->findProperty(name.Name)) {
            if (entry->type == entry_t::PROPERTY) {
                /////
                // Call the getter to read the property value.
                value = (this->*entry->_get)();
                return true;
            } else if (entry->type == entry_t::METHOD) {
                /////
                // TODO: A method is referenced.
                assert(false);
            }
        }
    }

    // check dynamic properties
    int index = getDynamicProperty(name);
    if (!(index < 0)) {
        value = dynamicValues[index];
        return true;
    }
    // check fixed properties
    index = getFixedProperty(name, current);
    if (!(index < 0)) {
        const abc::TraitInfo & trait = classDefinition->traits[index];
        if (trait.trait_type == abc::Trait_Slot) {
            value = fixedProperties[trait.index];
            return true;
        } else {
            // TODO: implement
            assert(false);
        }
    }

    /////
    // Follow the implicit prototype reference
    if (prototypeObject) {
        prototypeObject->Get(name, value);
    }
    return false;
}

bool CScriptObject::Get(const Object_MultiName & name,
                        Handle_t & value,
                        const abc::CAbcClassDefinition * current)
{
    if (name.Type == Object_MultiName::eQualifiedName) {
        return Get(name.qualifiedName, value, current);
    } else {
        // TODO: implement
        assert(false);
        return false;
    }
}

/**
 * Calls a named property.
 */
avm2::ErrorCode_t 
    CScriptObject::CallProperty(CVirtualMachine * vm,   // the virtual machine instance running the script
        const Object_MultiName & name,                  // the name of the property to invoke.
        const Handle_t args[],                          // the arguments to the method
        size_t numArgs,                                 // the number of arguments
        Handle_t & returnValue)                         // the return value of the method
{
    return Err_InvalidParameter;
}

bool CScriptObject::Put(const QName & name, 
                        Handle_t value,
                        bool addNewProperty,
                        const abc::CAbcClassDefinition * current)
{
    // first check embedded properties
    if (constructor) {
        if (const entry_t * entry = constructor->findProperty(name.Name)) {
            if (entry->type == entry_t::PROPERTY) {
                /////
                // Call the setter method with the value.
                if (entry->_set) {
                    (this->*entry->_set)(value);
                }
                return true;
            } else if (entry->type == entry_t::METHOD) {
                /////
                // A method is referenced. TODO: implement
                assert(false);
            }
        }
    }

    // check dynamic properties
    int index = getDynamicProperty(name);
    if (!(index < 0)) {
        dynamicValues[index] = value;
        return true;
    }
    // check fixed properties
    index = getFixedProperty(name, current);
    if (!(index < 0)) {
        const abc::TraitInfo & trait = classDefinition->traits[index];
        if (trait.trait_type == abc::Trait_Slot) {
            fixedProperties[trait.index] = value;
            return true;
        } else {
            // TODO: implement
            assert(false);
        }
    }

    if (prototypeObject) {
        if (prototypeObject->Put(name, value, false)) {
            return true;
        }
    }
    // add it as a new dynamic property
    if (addNewProperty) {
        dynamicValues.push_back(value);
        dynamicNames.push_back(name);
        return true;
    }
    return false;
}

bool CScriptObject::hasOwnProperty(const Object_MultiName & name,
                                   const abc::CAbcClassDefinition * current)
{
    // TODO: implement
    if (name.Type == Object_MultiName::eQualifiedName) {
        return hasOwnProperty(name.qualifiedName, current);
    } else {
        assert(false);
        return false;
    }
}

bool CScriptObject::hasOwnProperty(const QName & name,
                                   const abc::CAbcClassDefinition * current)
{
    // check embedded properties first
    if (constructor) {
        if (const entry_t * entry = constructor->findProperty(name.Name)) {
            return true;
        }
    }
    // check dynamic and fixed properties.
    if ((getDynamicProperty(name) >= 0) ||
        (getFixedProperty(name, current) >= 0))
    {
        return true;
    }
    return false;
}

bool CScriptObject::hasProperty(const Object_MultiName & name,
                                const abc::CAbcClassDefinition * current)
{
    if (name.Type == Object_MultiName::eQualifiedName) {
        return hasProperty(name.qualifiedName, current);
    } else {
        // TODO: implement
        assert(false);
        return false;
    }
}

bool CScriptObject::hasProperty(const QName & name,
                                const abc::CAbcClassDefinition * current)
{
    // check if this object has a matching property
    if (hasOwnProperty(name)) {
        return true;
    }
    // follow the prototype chain.
    if (prototypeObject) {
        return prototypeObject->hasProperty(name, current);
    }
    return false;
}

/**
 * Define the functions that are actually called by the virtual machine
 * to verify the parameters and invoke the correct method.
 */
static ErrorCode_t _hasOwnProperty(CVirtualMachine * vm,
                                   const std::vector<Handle_t> & a_Arguments,
                                   Handle_t & a_ReturnValue)
{
    return Err_NotImplemented;
}

static ErrorCode_t _toString(CVirtualMachine * vm,
                             const std::vector<Handle_t> & a_Arguments,
                             Handle_t & a_ReturnValue)
{
    return Err_NotImplemented;
}

std::string CScriptObject::toString() const
{
    return "";
}

bool CScriptObject::_has(const QName &)
{
    return false;
}

bool CScriptObject::_get(const QName &, Handle_t &)
{
    return false;
}

/**
 * Define the vtable that actually exports the methods to the script.
 */
DEFINE_VTABLE_BASE(CScriptObject)
    EXPORT_METHOD("hasOwnProperty", &_hasOwnProperty)
    EXPORT_METHOD("toString", &_toString)
    EXPORT_PROPERTY("constructor", &CScriptObject::getter_constructor, nullptr)
END_VTABLE()

//////
// Returns the value of the constructor property
avm2::Handle_t CScriptObject::getter_constructor(void)
{
    return Handle_t();
}

} // namespace avm2