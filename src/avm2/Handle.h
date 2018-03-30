#ifndef _HANDLE_H_
#define _HANDLE_H_

#include "Atom.h"

namespace avm2
{

class CScriptObject;

/**
 * Smart pointer class for the Atom values. As long as there exists a 
 * handle to a Atom, the object won't be deleted. This is required when handling
 * object that only exists on the native stack/heap and hasn't been added to the
 * actual AVM2 runtime yet.
 */
struct Handle_t
{
public:
    Handle_t();
    Handle_t(CScriptObject *);
    Handle_t(Atom_t);
    Handle_t(const Handle_t & _handle);
    ~Handle_t();

    const Atom_t & getValue() const { return value;}

    Handle_t & operator=(const Handle_t & rhs);
    Handle_t & operator=(Atom_t _value);

    operator Atom_t() const { return value; }

    CScriptObject * getObject() const { return toObject(value); }

private:

    void dereference();
    Atom_t              value;
};

#if 0
/**
 * Weak smart pointer
 */
struct WeakReference_t
{
    WeakReference_t();
    WeakReference_t(Object_t *);
    WeakReference_t(Atom_t);
    WeakReference_t(const WeakReference_t &);
    ~WeakReference_t();

    WeakReference_t & operator=(const WeakReference_t & rhs);
    WeakReference_t & operator=(Atom_t _value);
    operator Atom_t() const { return value; }

    Object_t * getObject() const { return toObject(value); }

private:
    void dereference();
    Atom_t              value;
};
#endif

}

#endif