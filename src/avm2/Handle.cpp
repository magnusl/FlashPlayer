#include "Handle.h"
#include "CScriptObject.h"
#include <assert.h>

namespace avm2
{
    Handle_t::Handle_t()
    {
        value   = 0;
    }

    void Handle_t::dereference()
    {
        if (CScriptObject * obj = toObject(value)) {
            obj->unlock();
        }
    }

    Handle_t::~Handle_t()
    {
        dereference();
    }

    Handle_t::Handle_t(CScriptObject * obj)
    {
        if (obj) {
            obj->lock();
        }
        value = objToAtom(obj); 
    }

    Handle_t::Handle_t(Atom_t _value)
    {
        if (CScriptObject * obj = toObject(_value)) {
            obj->lock();
        }
    }

    Handle_t::Handle_t(const Handle_t & _handle)
    {
        if (CScriptObject * obj = toObject(_handle.value)) {
            obj->lock();
        }
        value       = _handle.value;
    }

    Handle_t & Handle_t::operator=(const Handle_t & _rhs)
    {
        dereference();
        CScriptObject * obj = toObject(_rhs.value);
        if (obj) {
            obj->lock();
        }
        value   = _rhs.value;
        return *this;
    }

    Handle_t & Handle_t::operator=(Atom_t _value)
    {
        dereference();
        
        if (CScriptObject  * obj = toObject(_value)) {
            obj->lock();
        }
        value = _value;
        return *this;
    }


#if 0
/*****************************************************************************/
/*                                  Weak reference                           */
/*****************************************************************************/
WeakReference_t::WeakReference_t()
{
    value = 0;
}

WeakReference_t::WeakReference_t(Object_t * obj)
{
    if (obj) {
        obj->incWeakRef();
    }
    value = objToAtom(obj); 
}

WeakReference_t::WeakReference_t(Atom_t _value)
{
    if (Object_t * obj = toObject(_value)) {
        obj->incWeakRef();
    }
    value = _value;

}

WeakReference_t::WeakReference_t(const WeakReference_t & ref)
{
    if (Object_t * obj = toObject(ref.value)) {
        obj->incWeakRef();
    }
    value = ref.value;
}

WeakReference_t::~WeakReference_t()
{
    dereference();
}

WeakReference_t & WeakReference_t::operator=(const WeakReference_t & rhs)
{
    dereference();
    if (Object_t * obj = toObject(rhs.value)) {
        obj->incWeakRef();
    }
    value = rhs.value;
    return *this;
}

WeakReference_t & WeakReference_t::operator=(Atom_t _value)
{
    dereference();
    if (Object_t * obj = toObject(value)) {
        obj->incWeakRef();
    }
    value = value;
    return *this;
}

void WeakReference_t::dereference()
{
    if (Object_t * obj = toObject(value)) {
        obj->decWeakRef();
    }
}

#endif
}