#include "CEventDispatcher.h"
#include <map>
#include <assert.h>
#include <list>

using namespace std;
using namespace avm2;

namespace swf
{

/*****************************************************************************/
/*                                  Class                                    */
/*****************************************************************************/
CEventDispatcher_Class::CEventDispatcher_Class(CClassObject * cObject, 
                                               avm2::vtable_t * vt,
                                               CVirtualMachine & vm,
                                               CClassObject * baseType) :
CClassObject(cObject, vt, vm, baseType)
{
}

Handle_t CEventDispatcher_Class::construct(const std::vector<Handle_t> & arguments)
{
    return nullptr;
}

uint32_t CEventDispatcher_Class::getId() const
{
    return SWF_EVENTDISPATCHER;
}

DEFINE_VTABLE(CEventDispatcher_Class, CClassObject)
END_VTABLE()

/*****************************************************************************/
/*                                  Instance                                 */
/*****************************************************************************/
CEventDispatcher::CEventDispatcher(avm2::CClassObject * cObject, avm2::vtable_t * vt) : 
    CScriptObject(cObject, vt)
{
}

avm2::ErrorCode_t CEventDispatcher::addEventListener(avm2::CStringTable::index_t type, 
                                                     avm2::Handle_t listener,
                                                     bool useCapture)
{
    return Err_NotImplemented;
}

bool CEventDispatcher::hasEventListener(avm2::CStringTable::index_t type)
{
    return false;
}

/////
// GC
void CEventDispatcher::markReachable()  /**< marks any reachable resources */
{
    reachable = 1;
    CScriptObject::markReachable();
}

/////
// Exported properties
DEFINE_VTABLE(CEventDispatcher, CScriptObject)
END_VTABLE()

#if 0

struct EventHandler
{
    bool                useCapture;
    avm2::Object_t *    handler;
    avm2::CStringTable::index_t type;
};

// The internal representation of each created event dispatcher.
struct Component_t
{
    ~Component_t();
    std::list<EventHandler> handlers;
};

Component_t::~Component_t()
{
    for(std::list<EventHandler>::iterator it = handlers.begin();
        it != handlers.end();
        it++)
    {
        EventHandler & handler = *it;
        if (handler.handler) {
            --handler.handler->softref;
        }
    }
}

class CEventDispatcher_Impl
{
public:
    Component_t * getComponentById(size_t id);
    bool deleteComponentById(size_t id);

    std::map<uint32_t, Component_t> Objects;
};

Component_t * CEventDispatcher_Impl::getComponentById(size_t id)
{
    std::map<uint32_t, Component_t>::iterator it = Objects.find(id);
    return (it == Objects.end()) ? 0 : &it->second;
}

bool CEventDispatcher_Impl::deleteComponentById(size_t id)
{
    std::map<uint32_t, Component_t>::iterator it = Objects.find(id);
    if (it == Objects.end()) {
        return false;
    }
    Objects.erase(it);
    return true;
}

// constructor, initializes the class instance.
CEventDispatcher::CEventDispatcher(CClassObject * _baseClass, avm2::CVirtualMachine & vm) : 
    CClassObject(_baseClass, vm)
{
    impl = new (std::nothrow) CEventDispatcher_Impl;
}

// destructor, performs any required cleanup.
CEventDispatcher::~CEventDispatcher()
{
    delete impl;
}

// initializes a event dispatcher.
bool CEventDispatcher::initObject(avm2::Object_t * obj)
{
    assert(obj);
    if (impl->Objects.find(obj->id) != impl->Objects.end()) {
        // object already exists
        return false;
    }
    // initialize the base class.
    return baseClass ? baseClass->initObject(obj) : true;
}

bool CEventDispatcher::destroyObject(avm2::Object_t * obj)
{
    assert(obj);
    if (!impl->deleteComponentById(obj->id)) {
        return false;
    }
    return baseClass ? baseClass->destroyObject(obj) : true;
}

// registers a event listener
bool CEventDispatcher::_addEventListener(avm2::Object_t * obj,
                                         avm2::CStringTable::index_t type,
                                         avm2::Handle_t handler, 
                                         bool useCapture)
{
    Component_t * c = impl->getComponentById(obj->id);
    if (!c) { // not a event dispatcher
        return false;
    }
    return false;
}

// removes a event listener from a event dispatcher.
bool CEventDispatcher::_removeEventListener(avm2::Object_t * obj,
                                            avm2::CStringTable::index_t type,
                                            avm2::Handle_t handler, 
                                            bool useCapture)
{
    Component_t * c = impl->getComponentById(obj->id);
    if (!c) { // not a event dispatcher
        return false;
    }
    return false;
}

bool CEventDispatcher::hasProperty(avm2::Object_t * obj, const avm2::QName & name)
{
    return false;
}

bool CEventDispatcher::setProperty(avm2::Object_t * obj, const avm2::QName & name, avm2::Handle_t _value)
{
    return false;
}

bool CEventDispatcher::getProperty(avm2::Object_t * obj, const avm2::QName & name, avm2::Handle_t & _value)
{
    return false;
}

/*****************************************************************************/
/*                              Exported methods                             */
/*****************************************************************************/
DEFINE_VTABLE(CEventDispatcher, avm2::CScriptObject)
    EXPORT_METHOD("addEventListener", &swf::CEventDispatcher::addEventListener)
    EXPORT_METHOD("removeEventListener", &swf::CEventDispatcher::removeEventListener)
END_VTABLE()

avm2::ErrorCode_t CEventDispatcher::addEventListener(avm2::CVirtualMachine * vm,
    const std::vector<avm2::Handle_t> & a_Arguments,
    avm2::Handle_t & a_ReturnValue)
{
    return avm2::Err_Ok;
}

avm2::ErrorCode_t CEventDispatcher::removeEventListener(avm2::CVirtualMachine * vm,
    const std::vector<avm2::Handle_t> & a_Arguments,
    avm2::Handle_t & a_ReturnValue)
{
    return avm2::Err_Ok;
}
#endif
}