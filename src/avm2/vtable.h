#ifndef _VTABLE_H_
#define _VTABLE_H_

#include <vector>
#include "Atom.h"
#include "Handle.h"
#include "errorcode.h"
#include <stdint.h>

namespace avm2
{
    class CScriptObject;
    class CClassObject;
    class CVirtualMachine;

    //void (X::* ptfptr) (int)

    typedef avm2::Handle_t (CScriptObject::*getter_t) (void);
    typedef void (CScriptObject::*setter_t) (avm2::Handle_t);

    typedef ErrorCode_t (*call_t) (CVirtualMachine * vm, const std::vector<Handle_t> & a_Arguments, Handle_t & a_ReturnValue);

    struct entry_t
    {
        enum {
            END,
            METHOD,
            PROPERTY
        } type;

        getter_t _get;
        setter_t _set;
        call_t          _call;
        const char *    _name;
    };

    struct vtable_t {
        const vtable_t * next;
        entry_t * entries;
    };


/////
// Declares that the class has a vtable and thus exports methods or properties
#define DECLARE_VTABLE() \
public: \
static avm2::vtable_t vtable; \
static avm2::entry_t _entries[];

//////
// Macro fr exporting a method from a class
#define EXPORT_METHOD(methodName, methodPointer) \
    {avm2::entry_t::METHOD, nullptr, nullptr, methodPointer, methodName},

//////
// Macro for exporting access to a ActionScript 3 Object property
#define EXPORT_PROPERTY(propertyName, getter, setter) \
    {avm2::entry_t::PROPERTY, (avm2::getter_t) getter, (avm2::setter_t) setter, nullptr, propertyName},

/////
// Starts the definition of a vtable. Only used by the root class in the hierarchy
#define DEFINE_VTABLE_BASE(classname) \
    vtable_t classname::vtable = { nullptr, classname::_entries }; \
    entry_t classname::_entries[] = {

//////
// Ends a vtable definition.
#define END_VTABLE() {avm2::entry_t::END, nullptr, nullptr , nullptr, nullptr} };

/////
// Starts the definition of a vtable. 
#define DEFINE_VTABLE(classname, base) \
    avm2::vtable_t classname::vtable = { &base::vtable, classname::_entries }; \
    avm2::entry_t classname::_entries[] = {

} // namespace avm2

#endif